/**
 * StimulusDisplay.cpp
 *
 * History:
 * Dave Cox on ??/??/?? - Created.
 * Paul Jankunas on 05/23/05 - Fixed spacing.
 *
 * Copyright 2005 MIT.  All rights reserved.
 */

#include "OpenGLContextManager.h"
#include "Stimulus.h" 
#include "StimulusNode.h"
#include "Utilities.h"
#include "StandardVariables.h"
#include "Experiment.h"
#include "StandardVariables.h"
#include "ComponentRegistry.h"
#include "VariableNotification.h"

#include <CoreAudio/HostTime.h>

#include "boost/bind.hpp"


BEGIN_NAMESPACE_MW



/**********************************************************************
 *                  StimulusDisplay Methods
 **********************************************************************/
StimulusDisplay::StimulusDisplay() :
    refreshSync(2),
    currentOutputTimeUS(-1),
    current_context_index(-1)
{ }


void StimulusDisplay::initialize(){

    // defer creation of the display chain until after the stimulus display has been created
    display_stack = shared_ptr< LinkedList<StimulusNode> >(new LinkedList<StimulusNode>());
    
	setDisplayBounds();

    opengl_context_manager = OpenGLContextManager::instance();
    clock = Clock::instance();

    waitingForRefresh = false;
    needDraw = false;
    
    if (kCVReturnSuccess != CVDisplayLinkCreateWithActiveCGDisplays(&displayLink)) {
        throw SimpleException("Unable to create display link");
    }
        
    stateSystemNotification = shared_ptr<VariableCallbackNotification>(new VariableCallbackNotification(boost::bind(&StimulusDisplay::stateSystemCallback, this, _1, _2)));
    state_system_mode->addNotification(stateSystemNotification);
}

StimulusDisplay::~StimulusDisplay(){
    stateSystemNotification->remove();
    CVDisplayLinkRelease(displayLink);
}

void StimulusDisplay::setCurrent(int i){
    if ((i >= getNContexts()) || (i < 0)) {
        merror(M_DISPLAY_MESSAGE_DOMAIN, "invalid context index (%d)", i);
        return;
    }

	current_context_index = i;
	opengl_context_manager->setCurrent(context_ids[i]); 
}

shared_ptr<StimulusNode> StimulusDisplay::addStimulus(shared_ptr<Stimulus> stim) {
    if(!stim) {
        mprintf("Attempt to load NULL stimulus");
        return shared_ptr<StimulusNode>();
    }

	shared_ptr<StimulusNode> stimnode(new StimulusNode(stim));
	
    display_stack->addToFront(stimnode);
	
	return stimnode;
}

void StimulusDisplay::addStimulusNode(shared_ptr<StimulusNode> stimnode) {
    if(!stimnode) {
        mprintf("Attempt to load NULL stimulus");
        return;
    }
    
	// remove it, in case it already belongs to a list
	stimnode->remove();
	
	display_stack->addToFront(stimnode);  // TODO
}

void StimulusDisplay::setDisplayBounds(){
    shared_ptr<mw::ComponentRegistry> reg = mw::ComponentRegistry::getSharedRegistry();
    shared_ptr<Variable> main_screen_info = reg->getVariable(MAIN_SCREEN_INFO_TAGNAME);
  
    Datum display_info = *main_screen_info; // from standard variables
	if(display_info.getDataType() == M_DICTIONARY &&
	   display_info.hasKey(M_DISPLAY_WIDTH_KEY) &&
	   display_info.hasKey(M_DISPLAY_HEIGHT_KEY) &&
	   display_info.hasKey(M_DISPLAY_DISTANCE_KEY)){
	
		GLdouble width_unknown_units = display_info.getElement(M_DISPLAY_WIDTH_KEY);
		GLdouble height_unknown_units = display_info.getElement(M_DISPLAY_HEIGHT_KEY);
		GLdouble distance_unknown_units = display_info.getElement(M_DISPLAY_DISTANCE_KEY);
	
		GLdouble half_width_deg = (180. / M_PI) * atan((width_unknown_units/2.)/distance_unknown_units);
		GLdouble half_height_deg = half_width_deg * height_unknown_units / width_unknown_units;
		//GLdouble half_height_deg = (180. / M_PI) * atan((height_unknown_units/2.)/distance_unknown_units);
		
		left = -half_width_deg;
		right = half_width_deg;
		top = half_height_deg;
		bottom = -half_height_deg;
        
        degrees_per_screen_unit = 2.0 * half_width_deg / width_unknown_units;
	} else {
		left = M_STIMULUS_DISPLAY_LEFT_EDGE;
		right = M_STIMULUS_DISPLAY_RIGHT_EDGE;
		top = M_STIMULUS_DISPLAY_TOP_EDGE;
		bottom = M_STIMULUS_DISPLAY_BOTTOM_EDGE;
	}
	
	mprintf("Display bounds set to (%g left, %g right, %g top, %g bottom)",
			left, right, top, bottom);
}

void StimulusDisplay::getDisplayBounds(GLdouble &left, GLdouble &right, GLdouble &bottom, GLdouble &top) {
    left = this->left;
    right = this->right;
    bottom = this->bottom;
    top = this->top;
}

double StimulusDisplay::getMainDisplayRefreshRate() {
    double refreshRate = opengl_context_manager->getDisplayRefreshRate(opengl_context_manager->getMainDisplayIndex());
    if (refreshRate <= 0.0) {
        refreshRate = 60.0;
    }
    return refreshRate;
}

void StimulusDisplay::addContext(int _context_id){
	context_ids.push_back(_context_id);
}

void StimulusDisplay::getCurrentViewportSize(GLint &width, GLint &height) {
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    width = viewport[2];
    height = viewport[3];
}


void StimulusDisplay::stateSystemCallback(const Datum &data, MWorksTime time) {
    upgrade_lock lock(display_lock);

    int newState = data.getInteger();

    if (IDLE == newState) {
        
        if (CVDisplayLinkIsRunning(displayLink)) {
            // If another thread is waiting for a display refresh, allow it to complete before stopping
            // the display link
            while (waitingForRefresh) {
                refreshCond.wait(lock);
            }

            // We need to release the lock before calling CVDisplayLinkStop, because
            // StimulusDisplay::displayLinkCallback could be blocked waiting for the lock, and
            // CVDisplayLinkStop won't return until displayLinkCallback exits, leading to deadlock.
            lock.unlock();
            
            if (kCVReturnSuccess != CVDisplayLinkStop(displayLink)) {
                merror(M_DISPLAY_MESSAGE_DOMAIN, "Unable to stop display updates");
            } else {
                currentOutputTimeUS = -1;
                mprintf(M_DISPLAY_MESSAGE_DOMAIN, "Display updates stopped");
            }
        }
        
    } else if (RUNNING == newState) {

        if (!CVDisplayLinkIsRunning(displayLink)) {

            lastFrameTime = 0;

            if (kCVReturnSuccess != CVDisplayLinkSetOutputCallback(displayLink,
                                                                   &StimulusDisplay::displayLinkCallback,
                                                                   this) ||
                kCVReturnSuccess != opengl_context_manager->prepareDisplayLinkForMainDisplay(displayLink) ||
                kCVReturnSuccess != CVDisplayLinkStart(displayLink))
            {
                merror(M_DISPLAY_MESSAGE_DOMAIN, "Unable to start display updates");
            } else {
                // Wait for a refresh to complete, so we know that getCurrentOutputTimeUS() will return a valid time
                ensureRefresh(lock);
                
                mprintf(M_DISPLAY_MESSAGE_DOMAIN,
                        "Display updates started (main = %d, current = %d)",
                        CGMainDisplayID(),
                        CVDisplayLinkGetCurrentCGDisplay(displayLink));
            }

        }
        
    }
}


CVReturn StimulusDisplay::displayLinkCallback(CVDisplayLinkRef _displayLink,
                                              const CVTimeStamp *now,
                                              const CVTimeStamp *outputTime,
                                              CVOptionFlags flagsIn,
                                              CVOptionFlags *flagsOut,
                                              void *_display)
{
    StimulusDisplay *display = static_cast<StimulusDisplay*>(_display);
    
    {
        upgrade_lock upgradeLock(display->display_lock);
        
        {
            upgrade_to_unique_lock lock(upgradeLock);
            
            if (bool(warnOnSkippedRefresh->getValue())) {
                if (display->lastFrameTime) {
                    int64_t delta = (outputTime->videoTime - display->lastFrameTime) - outputTime->videoRefreshPeriod;
                    if (delta) {
                        mwarning(M_DISPLAY_MESSAGE_DOMAIN,
                                 "Skipped %g display refresh cycles",
                                 (double)delta / (double)(outputTime->videoRefreshPeriod));
                    }
                }
            }
            
            display->lastFrameTime = outputTime->videoTime;
            
            //
            // Here's how the time calculation works:
            //
            // outputTime->hostTime is the (estimated) time that the frame we're currently drawing will be displayed.
            // The value is in units of the "host time base", which appears to mean that it's directly comparable to
            // the value returned by mach_absolute_time().  However, the relationship to mach_absolute_time() is not
            // explicitly stated in the docs, so presumably we can't depend on it.
            //
            // What the CoreVideo docs *do* say is "the host time base for Core Video and the one for CoreAudio are
            // identical, and the values returned from either API can be used interchangeably".  Hence, we can use the
            // CoreAudio function AudioConvertHostTimeToNanos() to convert from the host time base to nanoseconds.
            //
            // Once we have a system time in nanoseconds, we substract the system base time and convert to
            // microseconds, which leaves us with a value that can be compared to clock->getCurrentTimeUS().
            //
            display->currentOutputTimeUS = (MWTime(AudioConvertHostTimeToNanos(outputTime->hostTime)) -
                                            display->clock->getSystemBaseTimeNS()) / 1000LL;
        }
        
        shared_lock sharedLock(upgradeLock);  // Downgrade to shared_lock
        display->refreshDisplay();
        display->waitingForRefresh = false;
    }
    
    // Signal waiting threads that refresh is complete
    display->refreshCond.notify_all();
    
    return kCVReturnSuccess;
}


void StimulusDisplay::refreshDisplay() {
    //
    // Determine whether we need to draw
    //
    
    if (!needDraw) {
        shared_ptr<StimulusNode> node = display_stack->getFrontmost();
        while (node) {
            if (node->isVisible()) {
                needDraw = node->needDraw();
                if (needDraw)
                    break;
            }
            node = node->getNext();
        }
    }
    
    if (!needDraw) {
        return;
    }

    //
    // Draw stimuli
    //
    
    if (context_ids.size() > 0) {
        for (int i = 0; i < context_ids.size(); i++) {
            setCurrent(i);
            
            if (i != 0) {
                
                // Non-main display
                drawDisplayStack(false);
                opengl_context_manager->updateAndFlush(i);
                
            } else {
                
                // Main display
                
                drawDisplayStack(true);
                opengl_context_manager->flush(i);
                if (opengl_context_manager->hasFence()) {
                    glSetFenceAPPLE(opengl_context_manager->getFence());
                }
                
                dispatch_async_f(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_HIGH, 0),
                                 this,
                                 &StimulusDisplay::announceDisplayUpdate);
                
            }
        }
        
        // Wait for StimulusDisplay::announceDisplayUpdate to acquire the lock
        refreshSync.wait();
    }
    
    needDraw = false;
}


void StimulusDisplay::clearDisplay() {
    upgrade_lock upgradeLock(display_lock);
    
    {
        upgrade_to_unique_lock lock(upgradeLock);
        
        shared_ptr<StimulusNode> node = display_stack->getFrontmost();
        while(node) {
            node->setVisible(false);
            node = node->getNext();
        }
    }
	
    needDraw = true;
    ensureRefresh(upgradeLock);
}


void StimulusDisplay::glInit() {

    glShadeModel(GL_FLAT);
    glDisable(GL_BLEND);
    glDisable(GL_DITHER);
    glDisable(GL_FOG);
    glDisable(GL_LIGHTING);
    
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL); // DDC added
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity(); // Reset The Projection Matrix
    
    gluOrtho2D(left, right, bottom, top);
    glMatrixMode(GL_MODELVIEW);
    
    glClearColor(0.5, 0.5, 0.5, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

}


void StimulusDisplay::drawDisplayStack(bool doStimAnnouncements) {
    // OpenGL setup

    glInit();
    
    // Draw all of the stimuli in the chain, back to front

    shared_ptr<StimulusNode> node = display_stack->getBackmost();
    while (node) {
        if (node->isVisible()) {
            node->draw(shared_from_this());
            
            if (doStimAnnouncements) {
                Datum individualAnnounce(node->getCurrentAnnounceDrawData());
                if (!individualAnnounce.isUndefined()) {
                    stimsToAnnounce.push_back(node);
                    stimAnnouncements.push_back(individualAnnounce);
                }
            }
        }
        node = node->getPrevious();
    }
}


void StimulusDisplay::updateDisplay() {
    upgrade_lock upgradeLock(display_lock);
    
    {
        upgrade_to_unique_lock lock(upgradeLock);
        
        shared_ptr<StimulusNode> node = display_stack->getFrontmost();
        while (node) {
            if (node->isPending()) {
                // we're taking care of the pending state, so
                // clear this flag
                node->clearPending();
                
                // convert "pending visible" stimuli to "visible" ones
                node->setVisible(node->isPendingVisible());
                
                if (node->isPendingRemoval()) {
                    node->clearPendingRemoval();
                    node->remove();
                    continue;
                }
            }
            
            node = node->getNext();
        }
    }
    
#define ERROR_ON_LATE_FRAMES
#ifdef ERROR_ON_LATE_FRAMES
    MWTime before_draw = clock->getCurrentTimeUS();
#endif

    needDraw = true;
    ensureRefresh(upgradeLock);
    
#ifdef ERROR_ON_LATE_FRAMES
    MWTime now = clock->getCurrentTimeUS();
    MWTime slop = 2 * (MWTime)(1000000.0 / getMainDisplayRefreshRate());
    
    if(now-before_draw > slop) {
        merror(M_DISPLAY_MESSAGE_DOMAIN,
               "updating main window display is taking longer than two frames (%lld > %lld) to update", 
               now-before_draw, 
               slop);		
    }
#endif
}


void StimulusDisplay::ensureRefresh(upgrade_lock &lock) {
    shared_lock sharedLock(lock);  // Downgrade to shared_lock
    
    if (!CVDisplayLinkIsRunning(displayLink)) {
        // Need to do the refresh here
        refreshDisplay();
    } else {
        // Wait for next display refresh to complete
        waitingForRefresh = true;
        do {
            refreshCond.wait(sharedLock);
        } while (waitingForRefresh);
    }
}


void StimulusDisplay::announceDisplayUpdate(void *_display) {
    StimulusDisplay *display = static_cast<StimulusDisplay*>(_display);
    shared_lock sharedLock(display->display_lock);
    
    // Wait for StimulusDisplay::refreshDisplay to finish drawing
    display->refreshSync.wait();
    
    display->setCurrent(0);
    if (display->opengl_context_manager->hasFence()) {
        if (glTestFenceAPPLE(display->opengl_context_manager->getFence())) {
            mwarning(M_DISPLAY_MESSAGE_DOMAIN,
                     "Display update announcement was delayed; time stamp on next %s event may be inaccurate",
                     STIMULUS_DISPLAY_UPDATE_TAGNAME);
        } else {
            glFinishFenceAPPLE(display->opengl_context_manager->getFence());
        }
    } else {
        glFinish();
    }
    
    MWTime now = display->clock->getCurrentTimeUS();
    stimDisplayUpdate->setValue(display->getAnnounceData(), now);
    display->announceDisplayStack(now);

    display->stimsToAnnounce.clear();
    display->stimAnnouncements.clear();
}


void StimulusDisplay::announceDisplayStack(MWTime time) {
    for (size_t i = 0; i < stimsToAnnounce.size(); i++) {
        stimsToAnnounce[i]->announce(stimAnnouncements[i], time);
    }
}


Datum StimulusDisplay::getAnnounceData() {
    Datum stimAnnounce(M_LIST, 1);
    for (size_t i = 0; i < stimAnnouncements.size(); i++) {
        stimAnnounce.addElement(stimAnnouncements[i]);
    }
	return stimAnnounce;
}


shared_ptr<StimulusDisplay> StimulusDisplay::getCurrentStimulusDisplay() {
    if (!GlobalCurrentExperiment) {
        throw SimpleException("no experiment currently defined");		
    }
    
    shared_ptr<StimulusDisplay> currentDisplay = GlobalCurrentExperiment->getStimulusDisplay();
    if (!currentDisplay) {
        throw SimpleException("no stimulus display in current experiment");
    }
    
    return currentDisplay;
}

// Delegated methods for transformations
// Default implementations are for orthographic display
void StimulusDisplay::translate2D(double x_deg, double y_deg){
    glTranslatef(x_deg, y_deg,0);
}

void StimulusDisplay::rotateInPlane2D(double rot_angle_deg){
    glRotatef(rot_angle_deg, 0,0,1);

}

void StimulusDisplay::scale2D(double x_size_deg, double y_size_deg){
    glScalef(x_size_deg, y_size_deg, 1.0);
}
        

void StimulusDisplay::translate2D_screenUnits(double x, double y){
    glTranslated(x * degrees_per_screen_unit, 
                 y * degrees_per_screen_unit, 
                 0.0);
}

void StimulusDisplay::scale2D_screenUnits(double x_size, double y_size){
    glTranslated(x_size * degrees_per_screen_unit, 
                 y_size * degrees_per_screen_unit, 
                 1.0);
}




VirtualTangentScreenDisplay::VirtualTangentScreenDisplay() : StimulusDisplay() { }


void VirtualTangentScreenDisplay::setDisplayBounds(){
    shared_ptr<mw::ComponentRegistry> reg = mw::ComponentRegistry::getSharedRegistry();
    shared_ptr<Variable> main_screen_info = reg->getVariable(MAIN_SCREEN_INFO_TAGNAME);
  
    Datum display_info = *main_screen_info; // from standard variables
	if(display_info.getDataType() == M_DICTIONARY &&
	   display_info.hasKey(M_DISPLAY_WIDTH_KEY) &&
	   display_info.hasKey(M_DISPLAY_HEIGHT_KEY) &&
	   display_info.hasKey(M_DISPLAY_DISTANCE_KEY)){
	

        if(display_info.hasKey(M_DISPLAY_X_OFFSET_KEY)){
            x_offset_screen_units = display_info.getElement(M_DISPLAY_X_OFFSET_KEY);
        } else {
            x_offset_screen_units = 0.0;
        }
        
        if(display_info.hasKey(M_DISPLAY_Y_OFFSET_KEY)){
            y_offset_screen_units = display_info.getElement(M_DISPLAY_Y_OFFSET_KEY);
        } else {
            y_offset_screen_units = 0.0;
        }        
    
		screen_width = display_info.getElement(M_DISPLAY_WIDTH_KEY);
		screen_height = display_info.getElement(M_DISPLAY_HEIGHT_KEY);
		screen_distance = display_info.getElement(M_DISPLAY_DISTANCE_KEY);
	
		fov_y_deg = 2 * (180. / M_PI) * atan((screen_height/2.)/screen_distance);
		fov_x_deg = 2 * (180. / M_PI) * atan((screen_width/2.)/screen_distance);
        
        screen_radius = sqrt( (screen_height * screen_height) / 4 +
                              (screen_width * screen_width) / 4 +
                              (screen_distance * screen_distance) );
                              
        screen_aspect_ratio = screen_width / screen_height;
        
        near_clip_distance = screen_distance - 1.0;
        far_clip_distance = screen_distance * 200.0;

        left = -fov_x_deg / 2.0;
        right = fov_x_deg / 2.0;
        top = fov_y_deg / 2.0;
        bottom = -fov_y_deg / 2.0;
        
	} else {
        throw SimpleException("Unable to compute screen dimensions");
	}

	
	mprintf("Display bounds set to (%g left, %g right, %g top, %g bottom)",
			left, right, top, bottom);
}

    
void VirtualTangentScreenDisplay::glInit(){ 


    glShadeModel(GL_SMOOTH);
    glEnable(GL_BLEND);
    glDisable(GL_DITHER);
    glDisable(GL_FOG);
    glDisable(GL_LIGHTING);
    
    glEnable (GL_POLYGON_SMOOTH);
    glHint (GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_MULTISAMPLE_ARB);
    
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity(); // Reset The Projection Matrix
    gluPerspective(fov_y_deg, screen_aspect_ratio, near_clip_distance, far_clip_distance);
    
    glMatrixMode(GL_MODELVIEW);
    
    glClearColor(0.5, 0.5, 0.5, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

}

void VirtualTangentScreenDisplay::translate2D(double x_deg, double y_deg){ 
    glTranslated(x_offset_screen_units, y_offset_screen_units, 0.0);
    glRotated(-x_deg, 0.0, 1.0, 0.0);
    glRotated(y_deg, 1.0, 0.0, 0.0);
    
    // Project out onto the virtual screen
    glTranslated(0.0, 0.0, -screen_radius);
}

void VirtualTangentScreenDisplay::rotateInPlane2D(double rot_angle_deg){ 
    glRotated(rot_angle_deg, 0.0, 0.0, 1.0);
}

void VirtualTangentScreenDisplay::scale2D(double x_size_deg, double y_size_deg){ 
    GLdouble xscale = 2.0 * screen_radius * tan(M_PI * x_size_deg / (2.0 * 180));
    GLdouble yscale = 2.0 * screen_radius * tan(M_PI * y_size_deg / (2.0 * 180));
    glScaled(xscale, yscale, 1.0);
} 

void VirtualTangentScreenDisplay::translate2D_screenUnits(double x, double y){
    glTranslated(x, y, -screen_distance);
}

void VirtualTangentScreenDisplay::scale2D_screenUnits(double x_size, double y_size){
    glScaled(x_size, y_size, 1.0);
}



VirtualTangentScreenDisplay::VirtualTangentScreenDisplay(const VirtualTangentScreenDisplay& s) :
    StimulusDisplay()
{ }


END_NAMESPACE_MW






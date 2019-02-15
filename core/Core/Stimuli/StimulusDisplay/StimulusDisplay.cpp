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
#include "StimulusNode.h"
#include "Utilities.h"
#include "StandardVariables.h"
#include "Experiment.h"
#include "ComponentRegistry.h"
#include "VariableNotification.h"


// A hack for now
#define M_STIMULUS_DISPLAY_LEFT_EDGE		-26.57
#define M_STIMULUS_DISPLAY_RIGHT_EDGE		26.57
#define M_STIMULUS_DISPLAY_BOTTOM_EDGE		-17.77
#define M_STIMULUS_DISPLAY_TOP_EDGE			17.77


BEGIN_NAMESPACE_MW


StimulusDisplay::StimulusDisplay(bool announceIndividualStimuli, bool useColorManagement) :
    current_context_index(-1),
    waitingForRefresh(false),
    needDraw(false),
    redrawOnEveryRefresh(false),
    projectionMatrix(GLKMatrix4Identity),
    displayUpdatesStarted(false),
    mainDisplayRefreshRate(0.0),
    currentOutputTimeUS(-1),
    announceIndividualStimuli(announceIndividualStimuli),
    announceStimuliOnImplicitUpdates(true),
    useColorManagement(useColorManagement)
{
    // defer creation of the display chain until after the stimulus display has been created
    display_stack = shared_ptr< LinkedList<StimulusNode> >(new LinkedList<StimulusNode>());
    
	setDisplayBounds();
    setBackgroundColor(0.5, 0.5, 0.5, 1.0);

    opengl_context_manager = OpenGLContextManager::instance();
    clock = Clock::instance();
    
    auto callback = [this](const Datum &data, MWorksTime time) {
        stateSystemCallback(data, time);
    };
    stateSystemNotification = boost::make_shared<VariableCallbackNotification>(callback);
    state_system_mode->addNotification(stateSystemNotification);
}

StimulusDisplay::~StimulusDisplay(){
    stateSystemNotification->remove();
}

OpenGLContextLock StimulusDisplay::setCurrent(int i){
    if ((i >= getNContexts()) || (i < 0)) {
        merror(M_DISPLAY_MESSAGE_DOMAIN, "invalid context index (%d)", i);
        return OpenGLContextLock();
    }

	return opengl_context_manager->setCurrent(context_ids[i]);
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

void StimulusDisplay::getDisplayBounds(const Datum &display_info,
                                       double &left,
                                       double &right,
                                       double &bottom,
                                       double &top)
{
	if(display_info.getDataType() == M_DICTIONARY &&
	   display_info.hasKey(M_DISPLAY_WIDTH_KEY) &&
	   display_info.hasKey(M_DISPLAY_HEIGHT_KEY) &&
	   display_info.hasKey(M_DISPLAY_DISTANCE_KEY)){
	
		double width_unknown_units = display_info.getElement(M_DISPLAY_WIDTH_KEY);
		double height_unknown_units = display_info.getElement(M_DISPLAY_HEIGHT_KEY);
		double distance_unknown_units = display_info.getElement(M_DISPLAY_DISTANCE_KEY);
	
		double half_width_deg = (180. / M_PI) * atan((width_unknown_units/2.)/distance_unknown_units);
		double half_height_deg = half_width_deg * height_unknown_units / width_unknown_units;
		//double half_height_deg = (180. / M_PI) * atan((height_unknown_units/2.)/distance_unknown_units);
		
		left = -half_width_deg;
		right = half_width_deg;
		top = half_height_deg;
		bottom = -half_height_deg;
	} else {
		left = M_STIMULUS_DISPLAY_LEFT_EDGE;
		right = M_STIMULUS_DISPLAY_RIGHT_EDGE;
		top = M_STIMULUS_DISPLAY_TOP_EDGE;
		bottom = M_STIMULUS_DISPLAY_BOTTOM_EDGE;
	}
}

void StimulusDisplay::setDisplayBounds(){
    shared_ptr<mw::ComponentRegistry> reg = mw::ComponentRegistry::getSharedRegistry();
    shared_ptr<Variable> main_screen_info = reg->getVariable(MAIN_SCREEN_INFO_TAGNAME);
    
    Datum display_info = *main_screen_info; // from standard variables
    getDisplayBounds(display_info, left, right, bottom, top);
    
    projectionMatrix = GLKMatrix4MakeOrtho(left, right, bottom, top, -1.0, 1.0);
	
	mprintf("Display bounds set to (%g left, %g right, %g top, %g bottom)",
			left, right, top, bottom);
}

void StimulusDisplay::getDisplayBounds(double &left, double &right, double &bottom, double &top) {
    left = this->left;
    right = this->right;
    bottom = this->bottom;
    top = this->top;
}

void StimulusDisplay::setBackgroundColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha) {
    backgroundRed = red;
    backgroundGreen = green;
    backgroundBlue = blue;
    backgroundAlpha = alpha;
}

void StimulusDisplay::setRedrawOnEveryRefresh(bool redrawOnEveryRefresh) {
    this->redrawOnEveryRefresh = redrawOnEveryRefresh;
}

void StimulusDisplay::setAnnounceStimuliOnImplicitUpdates(bool announceStimuliOnImplicitUpdates) {
    this->announceStimuliOnImplicitUpdates = announceStimuliOnImplicitUpdates;
}

void StimulusDisplay::addContext(int _context_id){
	context_ids.push_back(_context_id);
    int contextIndex = context_ids.size() - 1;
    
    auto ctxLock = opengl_context_manager->setCurrent(_context_id);
    prepareContext(contextIndex);
    
    if (0 == contextIndex) {
        setMainDisplayRefreshRate();
        allocateFramebufferStorage(contextIndex);
    }
}


void StimulusDisplay::allocateFramebufferStorage(int contextIndex) {
    glGenFramebuffers(1, &framebuffer);
    gl::FramebufferBinding<GL_DRAW_FRAMEBUFFER> framebufferBinding(framebuffer);
    
    GLint viewportWidth, viewportHeight;
    getCurrentViewportSize(viewportWidth, viewportHeight);
    framebufferTexture = opengl_context_manager->createFramebufferTexture(context_ids.at(contextIndex),
                                                                          viewportWidth,
                                                                          viewportHeight,
                                                                          useColorManagement);
    gl::TextureBinding<GL_TEXTURE_2D> textureBinding(framebufferTexture);
    
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferTexture, 0);
    
    if (GL_FRAMEBUFFER_COMPLETE != glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER)) {
        throw SimpleException("OpenGL framebuffer setup failed");
    }
}


void StimulusDisplay::getCurrentViewportSize(GLint &width, GLint &height) {
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    width = viewport[2];
    height = viewport[3];
}


void StimulusDisplay::pushFramebuffer(GLuint framebuffer, const std::vector<GLenum> &drawBuffers) {
    framebufferStack.emplace_back(framebuffer, drawBuffers);
    bindCurrentFramebuffer();
}


void StimulusDisplay::popFramebuffer() {
    if (framebufferStack.empty()) {
        merror(M_DISPLAY_MESSAGE_DOMAIN, "Internal error: No framebuffer to pop");
        return;
    }
    framebufferStack.pop_back();
    bindCurrentFramebuffer();
}


void StimulusDisplay::bindCurrentFramebuffer() {
    if (framebufferStack.empty()) {
        // Bind default framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return;
    }
    auto &framebufferInfo = framebufferStack.back();
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebufferInfo.first);
    glDrawBuffers(framebufferInfo.second.size(), framebufferInfo.second.data());
}


void StimulusDisplay::stateSystemCallback(const Datum &data, MWorksTime time) {
    unique_lock lock(display_lock);
    
    const int newStateSystemMode = data.getInteger();
    
    if ((RUNNING == newStateSystemMode) && !displayUpdatesStarted) {
        
        displayUpdatesStarted = true;  // Need to set this *before* calling startDisplayUpdates
        startDisplayUpdates();
        
        // Wait for a refresh to complete, so subclass methods that report the current
        // output time will return a valid value
        ensureRefresh(lock);
        
        mprintf(M_DISPLAY_MESSAGE_DOMAIN, "Display updates started (refresh rate: %g Hz)", getMainDisplayRefreshRate());
    
    } else if ((IDLE == newStateSystemMode) && displayUpdatesStarted) {
        
        // If another thread is waiting for a display refresh, allow it to complete before stopping
        // the display link
        while (waitingForRefresh) {
            refreshCond.wait(lock);
        }
        
        displayUpdatesStarted = false;  // Need to clear this *before* calling stopDisplayUpdates
        
        // We need to release the lock before calling stopDisplayUpdates, because
        // a display update callback could be blocked waiting for the lock, and
        // attempting to stop a blocked update loop could lead to deadlock
        lock.unlock();
        
        stopDisplayUpdates();
        currentOutputTimeUS = -1;
        
        mprintf(M_DISPLAY_MESSAGE_DOMAIN, "Display updates stopped");
        
    }
}


void StimulusDisplay::refreshMainDisplay() {
    //
    // Determine whether we need to draw
    //
    
    const bool updateIsExplicit = needDraw;
    needDraw = needDraw || redrawOnEveryRefresh;
    
    if (!needDraw) {
        shared_ptr<StimulusNode> node = display_stack->getFrontmost();
        while (node) {
            if (node->isVisible()) {
                needDraw = node->needDraw(shared_from_this());
                if (needDraw)
                    break;
            }
            node = node->getNext();
        }
    }

    //
    // Draw stimuli
    //
    
    constexpr int contextIndex = 0;
    const int context_id = context_ids.at(contextIndex);
    OpenGLContextLock ctxLock = opengl_context_manager->setCurrent(context_id);
    
    if (needDraw) {
        pushFramebuffer(framebuffer);
        
        current_context_index = contextIndex;
        drawDisplayStack();
        current_context_index = -1;
        
        popFramebuffer();
        opengl_context_manager->flushFramebufferTexture(context_id);
    }
    
    opengl_context_manager->drawFramebufferTexture(context_id);
    
    if (needDraw) {
        announceDisplayUpdate(updateIsExplicit);
    }
    
    needDraw = false;
}


void StimulusDisplay::refreshMirrorDisplay(int contextIndex) const {
    OpenGLContextLock ctxLock = opengl_context_manager->setCurrent(context_ids.at(contextIndex));
    opengl_context_manager->drawFramebufferTexture(context_ids.at(0), context_ids.at(contextIndex));
}


void StimulusDisplay::clearDisplay() {
    unique_lock lock(display_lock);
    
    // Remove all stimuli from the display stack
    while (auto node = display_stack->getFrontmost()) {
        node->remove();
    }
	
    needDraw = true;
    ensureRefresh(lock);
}


void StimulusDisplay::drawDisplayStack() {
#if !MWORKS_OPENGL_ES
    // This has no effect on non-sRGB framebuffers, so we can enable it unconditionally
    gl::Enabled<GL_FRAMEBUFFER_SRGB> framebufferSRGBEnabled;
#endif
    
    glDisable(GL_BLEND);
    glDisable(GL_DITHER);
    
    glClearColor(backgroundRed, backgroundGreen, backgroundBlue, backgroundAlpha);
    glClear(GL_COLOR_BUFFER_BIT);
    
    //
    // Draw all of the stimuli in the chain, back to front
    //
    shared_ptr<StimulusNode> node = display_stack->getBackmost();
    while (node) {
        if (node->isVisible()) {
            if (!(node->isLoaded())) {
                merror(M_DISPLAY_MESSAGE_DOMAIN,
                       "Stimulus \"%s\" is not loaded and will not be displayed",
                       node->getStimulus()->getTag().c_str());
            } else {
                node->draw(shared_from_this());
                
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


MWTime StimulusDisplay::updateDisplay() {
    unique_lock lock(display_lock);
    
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
                auto currentNode = node;
                node = currentNode->getNext();
                currentNode->remove();
                continue;
            }
        }
        
        node = node->getNext();
    }

    needDraw = true;
    ensureRefresh(lock);
    
    // Return the predicted output time of the just-submitted frame
    return currentOutputTimeUS;
}


void StimulusDisplay::ensureRefresh(unique_lock &lock) {
    if (!displayUpdatesStarted) {
        // Need to do the refresh here
        refreshMainDisplay();
        for (int i = 1; i < context_ids.size(); i++) {
            refreshMirrorDisplay(context_ids[i]);
        }
    } else {
        // Wait for next display refresh to complete
        waitingForRefresh = true;
        do {
            refreshCond.wait(lock);
        } while (waitingForRefresh);
    }
}


void StimulusDisplay::announceDisplayUpdate(bool updateIsExplicit) {
    MWTime now = getCurrentOutputTimeUS();
    if (-1 == now) {
        now = clock->getCurrentTimeUS();
    }
    
    stimDisplayUpdate->setValue(getAnnounceData(updateIsExplicit), now);
    
    if (announceIndividualStimuli && shouldAnnounceStimuli(updateIsExplicit)) {
        announceDisplayStack(now);
    }

    stimsToAnnounce.clear();
    stimAnnouncements.clear();
}


void StimulusDisplay::announceDisplayStack(MWTime time) {
    for (size_t i = 0; i < stimsToAnnounce.size(); i++) {
        stimsToAnnounce[i]->announce(stimAnnouncements[i], time);
    }
}


Datum StimulusDisplay::getAnnounceData(bool updateIsExplicit) {
    Datum stimAnnounce;
    
    if (!shouldAnnounceStimuli(updateIsExplicit)) {
        // No stim announcements, so just report the number of stimuli drawn
        stimAnnounce = Datum(long(stimAnnouncements.size()));
    } else {
        stimAnnounce = Datum(M_LIST, int(stimAnnouncements.size()));
        for (size_t i = 0; i < stimAnnouncements.size(); i++) {
            stimAnnounce.addElement(stimAnnouncements[i]);
        }
    }
    
	return stimAnnounce;
}


void StimulusDisplay::reportSkippedFrames(double numSkippedFrames) const {
    if (warnOnSkippedRefresh->getValue().getBool()) {
        mwarning(M_DISPLAY_MESSAGE_DOMAIN, "Skipped %g display refresh cycles", numSkippedFrames);
    }
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


END_NAMESPACE_MW




























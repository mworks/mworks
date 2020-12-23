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


StimulusDisplay::StimulusDisplay(bool useColorManagement) :
    opengl_context_manager(OpenGLContextManager::instance()),
    clock(Clock::instance()),
    main_context_id(NO_CONTEXT_ID),
    mirror_context_id(NO_CONTEXT_ID),
    display_stack(boost::make_shared<LinkedList<StimulusNode>>()),
    waitingForRefresh(false),
    needDraw(false),
    redrawOnEveryRefresh(false),
    projectionMatrix(GLKMatrix4Identity),
    backgroundRed(0.5),
    backgroundGreen(0.5),
    backgroundBlue(0.5),
    backgroundAlpha(1.0),
    displayUpdatesStarted(false),
    mainDisplayRefreshRate(0.0),
    currentOutputTimeUS(NO_CURRENT_OUTPUT_TIME),
    paused(false),
    didDrawWhilePaused(false),
    announceStimuliOnImplicitUpdates(true),
    useColorManagement(useColorManagement),
    framebuffer_id(-1)
{
    // Set display bounds
    auto reg = mw::ComponentRegistry::getSharedRegistry();
    getDisplayBounds(reg->getVariable(MAIN_SCREEN_INFO_TAGNAME)->getValue(),
                     boundsLeft, boundsRight, boundsBottom, boundsTop);
    projectionMatrix = GLKMatrix4MakeOrtho(boundsLeft, boundsRight, boundsBottom, boundsTop, -1.0, 1.0);
    mprintf("Display bounds set to (%g left, %g right, %g top, %g bottom)",
            boundsLeft, boundsRight, boundsTop, boundsBottom);
    
    // Register state system mode callback
    auto callback = [this](const Datum &data, MWorksTime time) {
        stateSystemCallback(data, time);
    };
    stateSystemNotification = boost::make_shared<VariableCallbackNotification>(callback);
    state_system_mode->addNotification(stateSystemNotification);
}


StimulusDisplay::~StimulusDisplay() {
    stateSystemNotification->remove();
}


OpenGLContextLock StimulusDisplay::setCurrent(int i) const {
    return opengl_context_manager->setCurrent(main_context_id);
}


void StimulusDisplay::addStimulusNode(const boost::shared_ptr<StimulusNode> &stimnode) {
    unique_lock lock(mutex);
    
    // Remove it first, in case it already belongs to a list
    stimnode->remove();
    display_stack->addToFront(stimnode);
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


void StimulusDisplay::setBackgroundColor(double red, double green, double blue, double alpha) {
    unique_lock lock(mutex);
    
    backgroundRed = red;
    backgroundGreen = green;
    backgroundBlue = blue;
    backgroundAlpha = alpha;
}


void StimulusDisplay::setRedrawOnEveryRefresh(bool value) {
    unique_lock lock(mutex);
    redrawOnEveryRefresh = value;
}


void StimulusDisplay::setAnnounceStimuliOnImplicitUpdates(bool value) {
    unique_lock lock(mutex);
    announceStimuliOnImplicitUpdates = value;
}


void StimulusDisplay::setMainContext(int context_id) {
    unique_lock lock(mutex);
    
    if (NO_CONTEXT_ID != main_context_id) {
        throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN, "Main context already set");
    }
    
    main_context_id = context_id;
    auto ctxLock = opengl_context_manager->setCurrent(context_id);
    prepareContext(context_id, true);
    framebuffer_id = createFramebuffer();
}


void StimulusDisplay::setMirrorContext(int context_id) {
    unique_lock lock(mutex);
    
    if (NO_CONTEXT_ID != mirror_context_id) {
        throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN, "Mirror context already set");
    }
    if (NO_CONTEXT_ID == main_context_id) {
        throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN, "Mirror context cannot be set before main context");
    }
    
    mirror_context_id = context_id;
    auto ctxLock = opengl_context_manager->setCurrent(context_id);
    prepareContext(context_id, false);
}


void StimulusDisplay::stateSystemCallback(const Datum &data, MWorksTime time) {
    unique_lock lock(mutex);
    
    switch (data.getInteger()) {
        case PAUSED:
            if (displayUpdatesStarted && !paused) {
                paused = true;
                didDrawWhilePaused = false;
            }
            break;
            
        case RUNNING:
            paused = false;
            if (!displayUpdatesStarted) {
                displayUpdatesStarted = true;  // Need to set this *before* calling startDisplayUpdates
                startDisplayUpdates();
                
                // Wait for a refresh to complete, so subclass methods that report the current
                // output time will return a valid value
                ensureRefresh(lock);
                
                mprintf(M_DISPLAY_MESSAGE_DOMAIN, "Display updates started (refresh rate: %g Hz)", getMainDisplayRefreshRate());
            }
            break;
            
        case IDLE:
            if (displayUpdatesStarted) {
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
                currentOutputTimeUS = NO_CURRENT_OUTPUT_TIME;
                
                mprintf(M_DISPLAY_MESSAGE_DOMAIN, "Display updates stopped");
            }
            break;
    }
}


void StimulusDisplay::refreshMainDisplay() {
    //
    // Determine whether we need to draw
    //
    
    const bool updateIsExplicit = needDraw;
    needDraw = needDraw || (redrawOnEveryRefresh && !(paused && didDrawWhilePaused));
    
    if (!needDraw) {
        auto node = display_stack->getFrontmost();
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
    
    OpenGLContextLock ctxLock = opengl_context_manager->setCurrent(main_context_id);
    
    if (needDraw) {
        pushFramebuffer(framebuffer_id);
        drawDisplayStack();
        popFramebuffer();
        if (paused) {
            didDrawWhilePaused = true;
        }
    }
    
    presentFramebuffer(framebuffer_id);
    
    if (needDraw) {
        announceDisplayUpdate(updateIsExplicit);
    }
    
    needDraw = false;
}


void StimulusDisplay::refreshMirrorDisplay() {
    OpenGLContextLock ctxLock = opengl_context_manager->setCurrent(mirror_context_id);
    presentFramebuffer(framebuffer_id, mirror_context_id);
}


void StimulusDisplay::clearDisplay() {
    unique_lock lock(mutex);
    
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
    auto node = display_stack->getBackmost();
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
                    stimAnnouncements.emplace_back(std::move(individualAnnounce));
                }
            }
        }
        node = node->getPrevious();
    }
}


MWTime StimulusDisplay::updateDisplay() {
    unique_lock lock(mutex);
    
    auto node = display_stack->getFrontmost();
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
        refreshMirrorDisplay();
    } else {
        // Wait for next display refresh to complete
        waitingForRefresh = true;
        do {
            refreshCond.wait(lock);
        } while (waitingForRefresh);
    }
}


void StimulusDisplay::announceDisplayUpdate(bool updateIsExplicit) {
    Datum stimAnnounce;
    if (!(updateIsExplicit || announceStimuliOnImplicitUpdates)) {
        // No stim announcements, so just report the number of stimuli drawn
        stimAnnounce = Datum(long(stimAnnouncements.size()));
    } else {
        stimAnnounce = Datum(std::move(stimAnnouncements));
    }
    stimAnnouncements.clear();
    
    MWTime now = getCurrentOutputTimeUS();
    if (NO_CURRENT_OUTPUT_TIME == now) {
        now = clock->getCurrentTimeUS();
    }
    
    stimDisplayUpdate->setValue(std::move(stimAnnounce), now);
}


void StimulusDisplay::reportSkippedFrames(double numSkippedFrames) const {
    if (warnOnSkippedRefresh->getValue().getBool()) {
        mwarning(M_DISPLAY_MESSAGE_DOMAIN,
                 "Skipped %g display refresh cycle%s",
                 numSkippedFrames,
                 (numSkippedFrames == 1.0 ? "" : "s"));
    }
}


boost::shared_ptr<StimulusDisplay> StimulusDisplay::getCurrentStimulusDisplay() {
    // Make a copy to ensure the experiment stays alive until we're done with it
    auto currentExperiment = GlobalCurrentExperiment;
    if (!currentExperiment) {
        throw SimpleException("No experiment currently defined");
    }
    
    auto currentDisplay = currentExperiment->getStimulusDisplay();
    if (!currentDisplay) {
        throw SimpleException("No stimulus display in current experiment");
    }
    
    return currentDisplay;
}


END_NAMESPACE_MW

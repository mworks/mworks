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
#define M_STIMULUS_DISPLAY_RIGHT_EDGE		 26.57
#define M_STIMULUS_DISPLAY_BOTTOM_EDGE		-17.77
#define M_STIMULUS_DISPLAY_TOP_EDGE			 17.77


BEGIN_NAMESPACE_MW


boost::shared_ptr<StimulusDisplay> StimulusDisplay::getDefaultStimulusDisplay() {
    // Make a copy to ensure the experiment stays alive until we're done with it
    auto currentExperiment = GlobalCurrentExperiment;
    if (!currentExperiment) {
        throw SimpleException("No experiment currently defined");
    }
    
    auto defaultDisplay = currentExperiment->getDefaultStimulusDisplay();
    if (!defaultDisplay) {
        throw SimpleException("No default stimulus display in current experiment");
    }
    
    return defaultDisplay;
}


void StimulusDisplay::getDisplayBounds(const Datum &display_info,
                                       double &left,
                                       double &right,
                                       double &bottom,
                                       double &top)
{
    if (display_info.isDictionary() &&
        display_info.hasKey(M_DISPLAY_WIDTH_KEY) &&
        display_info.hasKey(M_DISPLAY_HEIGHT_KEY) &&
        display_info.hasKey(M_DISPLAY_DISTANCE_KEY))
    {
        double width_unknown_units = display_info.getElement(M_DISPLAY_WIDTH_KEY);
        double height_unknown_units = display_info.getElement(M_DISPLAY_HEIGHT_KEY);
        double distance_unknown_units = display_info.getElement(M_DISPLAY_DISTANCE_KEY);
        
        double half_width_deg = (180.0 / M_PI) * std::atan((width_unknown_units / 2.0) / distance_unknown_units);
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


StimulusDisplay::StimulusDisplay(bool useColorManagement) :
    contextManager(OpenGLContextManager::instance()),
    clock(Clock::instance()),
    useColorManagement(useColorManagement),
    backgroundRed(0.5),
    backgroundGreen(0.5),
    backgroundBlue(0.5),
    backgroundAlpha(1.0),
    redrawOnEveryRefresh(false),
    announceStimuliOnImplicitUpdates(true),
    main_context_id(NO_CONTEXT_ID),
    mirror_context_id(NO_CONTEXT_ID),
    mainDisplayRefreshRate(0.0),
    displayStack(boost::make_shared<LinkedList<StimulusNode>>()),
    displayUpdatesStarted(false),
    currentOutputTimeUS(NO_CURRENT_OUTPUT_TIME),
    paused(false),
    didDrawWhilePaused(false)
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


void StimulusDisplay::setBackgroundColor(double red, double green, double blue, double alpha) {
    lock_guard lock(mutex);
    std::tie(backgroundRed, backgroundGreen, backgroundBlue, backgroundAlpha) = std::tie(red, green, blue, alpha);
}


void StimulusDisplay::setRedrawOnEveryRefresh(bool value) {
    lock_guard lock(mutex);
    redrawOnEveryRefresh = value;
}


void StimulusDisplay::setAnnounceStimuliOnImplicitUpdates(bool value) {
    lock_guard lock(mutex);
    announceStimuliOnImplicitUpdates = value;
}


void StimulusDisplay::setMainContext(int context_id) {
    lock_guard lock(mutex);
    
    if (NO_CONTEXT_ID != main_context_id) {
        throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN, "Main context already set");
    }
    
    main_context_id = context_id;
    prepareContext(context_id, true);
}


void StimulusDisplay::setMirrorContext(int context_id) {
    lock_guard lock(mutex);
    
    if (NO_CONTEXT_ID != mirror_context_id) {
        throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN, "Mirror context already set");
    }
    if (NO_CONTEXT_ID == main_context_id) {
        throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN, "Mirror context cannot be set before main context");
    }
    
    mirror_context_id = context_id;
    prepareContext(context_id, false);
}


void StimulusDisplay::addStimulusNode(const boost::shared_ptr<StimulusNode> &stimnode) {
    lock_guard lock(mutex);
    
    // Remove it first, in case it already belongs to a list
    stimnode->remove();
    displayStack->addToFront(stimnode);
}


OpenGLContextLock StimulusDisplay::setCurrentOpenGLContext() const {
    return contextManager->setCurrent(main_context_id);
}


void StimulusDisplay::refreshDisplay() {
    bool needDraw = false;
    
    //
    // Perform pending actions
    //
    
    std::vector<boost::shared_ptr<UpdateInfo>> pendingUpdateInfo;
    {
        lock_guard lock(pendingActionsMutex);
        
        for (auto &action : pendingActions) {
            switch (action.first) {
                case Action::Update: {
                    auto node = displayStack->getFrontmost();
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
                    break;
                }
                    
                case Action::Clear: {
                    // Remove all stimuli from the display stack
                    while (auto node = displayStack->getFrontmost()) {
                        node->clearPending();
                        node->clearPendingRemoval();
                        node->setVisible(false);
                        node->remove();
                    }
                    needDraw = true;
                    break;
                }
            }
            
            pendingUpdateInfo.emplace_back(std::move(action.second));
        }
        
        pendingActions.clear();
    }
    
    //
    // Determine whether we need to draw
    //
    
    const bool updateIsExplicit = needDraw;
    needDraw = needDraw || (redrawOnEveryRefresh && !(paused && didDrawWhilePaused));
    
    if (!needDraw) {
        auto node = displayStack->getFrontmost();
        auto sharedThis = shared_from_this();
        while (node) {
            if (node->isVisible()) {
                needDraw = node->needDraw(sharedThis);
                if (needDraw)
                    break;
            }
            node = node->getNext();
        }
    }
    
    //
    // Collect stimuli to draw, back to front
    //
    
    std::vector<boost::shared_ptr<Stimulus>> stimsToDraw;
    
    if (needDraw) {
        auto node = displayStack->getBackmost();
        while (node) {
            if (node->isVisible()) {
                auto stim = node->getStimulus();
                if (stim->isLoaded()) {
                    stimsToDraw.emplace_back(std::move(stim));
                } else {
                    merror(M_DISPLAY_MESSAGE_DOMAIN,
                           "Stimulus \"%s\" is not loaded and will not be displayed",
                           stim->getTag().c_str());
                }
            }
            node = node->getPrevious();
        }
    }
    
    //
    // Draw stimuli
    //
    
    renderDisplay(needDraw, stimsToDraw);
    if (needDraw && paused) {
        didDrawWhilePaused = true;
    }
    
    //
    // Announce draw
    //
    
    if (needDraw) {
        auto displayUpdateTime = getEventTimeForCurrentOutputTime();
        
        Datum stimAnnounce;
        if (!(updateIsExplicit || announceStimuliOnImplicitUpdates)) {
            // No stim announcements, so just report the number of stimuli drawn
            stimAnnounce = Datum(long(stimsToDraw.size()));
        } else {
            std::vector<Datum> stimAnnouncements;
            for (auto &stim : stimsToDraw) {
                stimAnnouncements.emplace_back(stim->getCurrentAnnounceDrawData());
            }
            stimAnnounce = Datum(std::move(stimAnnouncements));
        }
        stimDisplayUpdate->setValue(std::move(stimAnnounce), displayUpdateTime);
        
        for (auto &updateInfo : pendingUpdateInfo) {
            updateInfo->setPredictedOutputTime(displayUpdateTime);
        }
    }
}


void StimulusDisplay::reportSkippedFrames(double numSkippedFrames) const {
    if (warnOnSkippedRefresh->getValue().getBool()) {
        mwarning(M_DISPLAY_MESSAGE_DOMAIN,
                 "Skipped %g display refresh cycle%s",
                 numSkippedFrames,
                 (numSkippedFrames == 1.0 ? "" : "s"));
    }
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
                
                // Release the lock and sleep a bit (5 refresh periods seems sufficient) to let the
                // display link thread "warm up" and get synced with the display refresh cycle.
                // This also ensures that subclass methods that report the current output time will
                // return a valid value.
                lock.unlock();
                clock->sleepMS(5000.0 / getMainDisplayRefreshRate());
                
                mprintf(M_DISPLAY_MESSAGE_DOMAIN,
                        "Display updates started (refresh rate: %g Hz)",
                        getMainDisplayRefreshRate());
            }
            break;
            
        case IDLE:
            if (displayUpdatesStarted) {
                // Release any threads that are waiting for a display refresh
                {
                    lock_guard lock(pendingActionsMutex);
                    for (auto &action : pendingActions) {
                        action.second->setPredictedOutputTime(0);
                    }
                    pendingActions.clear();
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


auto StimulusDisplay::ensureRefresh(Action action) -> boost::shared_ptr<UpdateInfo> {
    auto updateInfo = boost::make_shared<UpdateInfo>();
    
    {
        lock_guard lock(pendingActionsMutex);
        pendingActions.emplace_back(action, updateInfo);
    }
    
    if (!displayUpdatesStarted) {
        // Need to do the refresh here
        lock_guard lock(mutex);
        refreshDisplay();
    }
    
    return updateInfo;
}


END_NAMESPACE_MW

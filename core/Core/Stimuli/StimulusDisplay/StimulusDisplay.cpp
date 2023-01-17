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


template <typename T>
static inline void getConfigValue(const Datum::dict_value_type &infoDict, const char *key, T &value) {
    auto iter = infoDict.find(Datum(key));
    if (iter != infoDict.end()) {
        value = T(iter->second);
    }
}


auto StimulusDisplay::getDisplayConfiguration(const Datum &displayInfo) -> Configuration {
    Configuration config;
    if (displayInfo.isDictionary()) {
        auto &infoDict = displayInfo.getDict();
        getConfigValue(infoDict, M_DISPLAY_TO_USE_KEY, config.displayToUse);
        getConfigValue(infoDict, M_DISPLAY_WIDTH_KEY, config.width);
        getConfigValue(infoDict, M_DISPLAY_HEIGHT_KEY, config.height);
        getConfigValue(infoDict, M_DISPLAY_DISTANCE_KEY, config.distance);
        getConfigValue(infoDict, M_REFRESH_RATE_KEY, config.refreshRateHz);
        getConfigValue(infoDict, M_ALWAYS_DISPLAY_MIRROR_WINDOW_KEY, config.alwaysDisplayMirrorWindow);
        getConfigValue(infoDict, M_MIRROR_WINDOW_BASE_HEIGHT_KEY, config.mirrorWindowBaseHeight);
        getConfigValue(infoDict, M_USE_COLOR_MANAGEMENT_KEY, config.useColorManagement);
        getConfigValue(infoDict, M_SET_DISPLAY_GAMMA_KEY, config.setDisplayGamma);
        getConfigValue(infoDict, M_DISPLAY_GAMMA_RED_KEY, config.redGamma);
        getConfigValue(infoDict, M_DISPLAY_GAMMA_GREEN_KEY, config.greenGamma);
        getConfigValue(infoDict, M_DISPLAY_GAMMA_BLUE_KEY, config.blueGamma);
        getConfigValue(infoDict, M_MAKE_WINDOW_OPAQUE_KEY, config.makeWindowOpaque);
    }
    return config;
}


boost::shared_ptr<StimulusDisplay> StimulusDisplay::prepareStimulusDisplay(const Configuration &config) {
    auto contextManager = OpenGLContextManager::instance(false);
    if (!contextManager) {
        contextManager = OpenGLContextManager::createPlatformOpenGLContextManager();
        OpenGLContextManager::registerInstance(contextManager);
    }
    
    if (config.displayToUse >= contextManager->getNumDisplays()) {
        throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN,
                              boost::format("Requested display index (%1%) is invalid")
                              % config.displayToUse);
    }
    if (config.width <= 0.0 || config.height <= 0.0 || config.distance <= 0.0) {
        throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN,
                              "Stimulus display width, height, and distance must be positive numbers");
    }
    double mirrorWindowWidth = 0.0;
    if (config.displayToUse < 0 || config.alwaysDisplayMirrorWindow) {
        if (config.mirrorWindowBaseHeight <= 0.0) {
            throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN,
                                  "Stimulus display mirror window height must be a positive number");
        }
        mirrorWindowWidth = config.mirrorWindowBaseHeight * (config.width / config.height);
    }
    
    auto display = createPlatformStimulusDisplay(config);
    
    if (config.displayToUse < 0) {
        // Mirror window only
        display->setMainContext(contextManager->createMirrorContext(mirrorWindowWidth, config.mirrorWindowBaseHeight));
    } else {
        auto mainContext = contextManager->createFullscreenContext(config.displayToUse, config.makeWindowOpaque);
        display->setMainContext(mainContext);
        if (config.alwaysDisplayMirrorWindow) {
            display->setMirrorContext(contextManager->createMirrorContext(mirrorWindowWidth,
                                                                          config.mirrorWindowBaseHeight,
                                                                          mainContext));
        }
    }
    
    display->clearDisplay();
    
    return display;
}


boost::shared_ptr<StimulusDisplay> StimulusDisplay::getDefaultStimulusDisplay() {
    // Make a copy to ensure the experiment stays alive until we're done with it
    auto experiment = GlobalCurrentExperiment;
    if (!experiment) {
        throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN, "No experiment currently defined");
    }
    // getDefaultStimulusDisplay will either succeed or throw, so we don't need to check the result
    return experiment->getDefaultStimulusDisplay();
}


void StimulusDisplay::getDisplayBounds(double width,
                                       double height,
                                       double distance,
                                       double &left,
                                       double &right,
                                       double &bottom,
                                       double &top)
{
    const double half_width_deg = (180.0 / M_PI) * std::atan((width / 2.0) / distance);
    const double half_height_deg = half_width_deg * height / width;
    left = -half_width_deg;
    right = half_width_deg;
    bottom = -half_height_deg;
    top = half_height_deg;
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
        const auto width = display_info.getElement(M_DISPLAY_WIDTH_KEY).getFloat();
        const auto height = display_info.getElement(M_DISPLAY_HEIGHT_KEY).getFloat();
        const auto distance = display_info.getElement(M_DISPLAY_DISTANCE_KEY).getFloat();
        getDisplayBounds(width, height, distance, left, right, bottom, top);
    } else {
        left = M_STIMULUS_DISPLAY_LEFT_EDGE;
        right = M_STIMULUS_DISPLAY_RIGHT_EDGE;
        bottom = M_STIMULUS_DISPLAY_BOTTOM_EDGE;
        top = M_STIMULUS_DISPLAY_TOP_EDGE;
    }
}


StimulusDisplay::StimulusDisplay(const Configuration &config) :
    contextManager(OpenGLContextManager::instance()),
    clock(Clock::instance()),
    config(config),
    backgroundRed(0.5),
    backgroundGreen(0.5),
    backgroundBlue(0.5),
    backgroundAlpha(1.0),
    redrawOnEveryRefresh(false),
    announceStimuliOnImplicitUpdates(true),
    updateVar(stimDisplayUpdate),
    captureVar(stimDisplayCapture),
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
    getDisplayBounds(config.width, config.height, config.distance,
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


void StimulusDisplay::setUpdateVar(const boost::shared_ptr<Variable> &var) {
    lock_guard lock(mutex);
    updateVar = var;
}


void StimulusDisplay::setCaptureVar(const boost::shared_ptr<Variable> &var) {
    lock_guard lock(mutex);
    captureVar = var;
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
        updateVar->setValue(std::move(stimAnnounce), displayUpdateTime);
        
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

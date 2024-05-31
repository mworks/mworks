/*
 *  StimulusDisplay.h
 *  MWorksCore
 *
 *  Created by David Cox on 6/10/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef STIMULUS_DISPLAY_H_
#define STIMULUS_DISPLAY_H_

#include <map>
#include <vector>

#include <GLKit/GLKMath.h>

#include <boost/noncopyable.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "Clock.h"
#include "FairMutex.hpp"
#include "LinkedList.h"


BEGIN_NAMESPACE_MW


// Forward declarations
class Datum;
class OpenGLContextManager;
class Stimulus;
class StimulusNode;
class Variable;
class VariableCallbackNotification;


class StimulusDisplay : public boost::enable_shared_from_this<StimulusDisplay>, boost::noncopyable {
    
public:
    struct Configuration {
        int displayToUse = 0;
        double width = 0.0, height = 0.0, distance = 0.0;
        double refreshRateHz = 60.0;
        bool alwaysDisplayMirrorWindow = false;
        double mirrorWindowBaseHeight = 0.0;
        bool useColorManagement = true;
        bool setDisplayGamma = false;
        double redGamma = 0.0, greenGamma = 0.0, blueGamma = 0.0;
        bool makeWindowOpaque = true;
    };
    
    struct UpdateInfo {
        UpdateInfo() : predictedOutputTime(-1) { }
        
        bool isPending() const { return (predictedOutputTime < 0); }
        MWTime getPredictedOutputTime() const { return predictedOutputTime; }
        void setPredictedOutputTime(MWTime time) { predictedOutputTime = time; }
        
    private:
        std::atomic<MWTime> predictedOutputTime;
        static_assert(decltype(predictedOutputTime)::is_always_lock_free);
    };
    
    static Configuration getDisplayConfiguration(const Datum &displayInfo);
    static boost::shared_ptr<StimulusDisplay> prepareStimulusDisplay(const Configuration &config);
    static boost::shared_ptr<StimulusDisplay> createPlatformStimulusDisplay(const Configuration &config);
    static boost::shared_ptr<StimulusDisplay> getDefaultStimulusDisplay();
    
    static void getDisplayBounds(double width, double height, double distance,
                                 double &left, double &right, double &bottom, double &top);
    static void getDisplayBounds(const Datum &mainScreenInfo, double &left, double &right, double &bottom, double &top);
    
    explicit StimulusDisplay(const Configuration &config);
    virtual ~StimulusDisplay();
    
    bool getUseColorManagement() const { return config.useColorManagement; }
    void getDisplayBounds(double &left, double &right, double &bottom, double &top) const {
        std::tie(left, right, bottom, top) = std::tie(boundsLeft, boundsRight, boundsBottom, boundsTop);
    }
    GLKMatrix4 getProjectionMatrix() const { return projectionMatrix; }
    double getMainDisplayRefreshRate() const { return mainDisplayRefreshRate; }
    MWTime getCurrentOutputTimeUS() const { return currentOutputTimeUS; }
    
    void setBackgroundColor(double red, double green, double blue, double alpha);
    void setRedrawOnEveryRefresh(bool value);
    void setAnnounceStimuliOnImplicitUpdates(bool value);
    void setUpdateVar(const boost::shared_ptr<Variable> &var);
    void setCaptureVar(const boost::shared_ptr<Variable> &var);
    
    void setMainContext(int context_id);
    void setMirrorContext(int context_id);
    void addStimulusNode(const boost::shared_ptr<StimulusNode> &stimnode);
    
    boost::shared_ptr<UpdateInfo> updateDisplay() { return ensureRefresh(Action::Update); }
    boost::shared_ptr<UpdateInfo> clearDisplay() { return ensureRefresh(Action::Clear); }
    
    virtual void configureCapture(const std::string &format,
                                  int heightPixels,
                                  const boost::shared_ptr<Variable> &enabled) = 0;
    
protected:
    const boost::shared_ptr<OpenGLContextManager> & getContextManager() const { return contextManager; }
    const boost::shared_ptr<Clock> & getClock() const { return clock; }
    const Configuration & getConfiguration() const { return config; }
    void getBackgroundColor(double &red, double &green, double &blue, double &alpha) const {
        std::tie(red, green, blue, alpha) = std::tie(backgroundRed, backgroundGreen, backgroundBlue, backgroundAlpha);
    }
    const boost::shared_ptr<Variable> & getCaptureVar() const { return captureVar; }
    bool getDisplayUpdatesStarted() const { return displayUpdatesStarted; }
    MWTime getEventTimeForCurrentOutputTime() const {
        return (currentOutputTimeUS == NO_CURRENT_OUTPUT_TIME ? clock->getCurrentTimeUS() : getCurrentOutputTimeUS());
    }
    
    void setMainDisplayRefreshRate(double value) { mainDisplayRefreshRate = value; }
    void setCurrentOutputTimeUS(MWTime value) { currentOutputTimeUS = value; }
    
    void refreshDisplay();
    void reportSkippedFrames(double numSkippedFrames) const;
    
    virtual void prepareContext(int context_id, bool isMainContext) = 0;
    virtual void startDisplayUpdates() = 0;
    virtual void stopDisplayUpdates() = 0;
    virtual void renderDisplay(bool needDraw, const std::vector<boost::shared_ptr<Stimulus>> &stimsToDraw) = 0;
    
    using unique_lock = std::unique_lock<FairMutex>;
    using lock_guard = std::lock_guard<unique_lock::mutex_type>;
    lock_guard::mutex_type mutex;
    
private:
    enum class Action {
        Update,
        Clear
    };
    
    void stateSystemCallback(const Datum &data, MWorksTime time);
    boost::shared_ptr<UpdateInfo> ensureRefresh(Action action);
    
    const boost::shared_ptr<OpenGLContextManager> contextManager;
    const boost::shared_ptr<Clock> clock;
    
    const Configuration config;
    double backgroundRed, backgroundGreen, backgroundBlue, backgroundAlpha;  // background color
    bool redrawOnEveryRefresh;
    bool announceStimuliOnImplicitUpdates;
    boost::shared_ptr<Variable> updateVar;
    boost::shared_ptr<Variable> captureVar;
    
    double boundsLeft, boundsRight, boundsBottom, boundsTop;  // display bounds
    GLKMatrix4 projectionMatrix;
    
    boost::shared_ptr<VariableCallbackNotification> stateSystemNotification;
    
    static constexpr int NO_CONTEXT_ID = -1;
    int main_context_id;
    int mirror_context_id;
    
    double mainDisplayRefreshRate;
    
    boost::shared_ptr<LinkedList<StimulusNode>> displayStack;
    
    std::atomic_bool displayUpdatesStarted;
    static_assert(decltype(displayUpdatesStarted)::is_always_lock_free);
    
    static constexpr MWTime NO_CURRENT_OUTPUT_TIME = -1;
    std::atomic<MWTime> currentOutputTimeUS;
    static_assert(decltype(currentOutputTimeUS)::is_always_lock_free);
    
    bool paused;
    bool didDrawWhilePaused;
    
    std::vector<std::pair<Action, boost::shared_ptr<UpdateInfo>>> pendingActions;
    lock_guard::mutex_type pendingActionsMutex;
    
};


using StimulusDisplayPtr = boost::shared_ptr<StimulusDisplay>;


END_NAMESPACE_MW


#endif

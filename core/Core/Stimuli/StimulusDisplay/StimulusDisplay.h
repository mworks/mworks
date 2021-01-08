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
#include "LinkedList.h"
#include "OpenGLContextLock.h"


BEGIN_NAMESPACE_MW


// Forward declarations
class Datum;
class OpenGLContextManager;
class Stimulus;
class StimulusNode;
class VariableCallbackNotification;


enum class RenderingMode {
    OpenGL,
    Metal,
    None
};


class StimulusDisplay : public boost::enable_shared_from_this<StimulusDisplay>, boost::noncopyable {
    
public:
    static boost::shared_ptr<StimulusDisplay> createPlatformStimulusDisplay(bool useColorManagement);
    static boost::shared_ptr<StimulusDisplay> getCurrentStimulusDisplay();
    
    static void getDisplayBounds(const Datum &mainScreenInfo, double &left, double &right, double &bottom, double &top);
    
    explicit StimulusDisplay(bool useColorManagement);
    virtual ~StimulusDisplay();
    
    bool getUseColorManagement() const { return useColorManagement; }
    void getDisplayBounds(double &left, double &right, double &bottom, double &top) const {
        std::tie(left, right, bottom, top) = std::tie(boundsLeft, boundsRight, boundsBottom, boundsTop);
    }
    GLKMatrix4 getProjectionMatrix() const { return projectionMatrix; }
    double getMainDisplayRefreshRate() const { return mainDisplayRefreshRate; }
    MWTime getCurrentOutputTimeUS() const { return currentOutputTimeUS; }
    
    void setBackgroundColor(double red, double green, double blue, double alpha);
    void setRedrawOnEveryRefresh(bool value);
    void setAnnounceStimuliOnImplicitUpdates(bool value);
    
    void setMainContext(int context_id);
    void setMirrorContext(int context_id);
    void addStimulusNode(const boost::shared_ptr<StimulusNode> &stimnode);
    
    MWTime updateDisplay();
    void clearDisplay();
    
    virtual void setRenderingMode(RenderingMode mode) = 0;
    
    virtual int createFramebuffer() = 0;
    virtual void pushFramebuffer(int framebuffer_id) = 0;
    virtual void bindCurrentFramebuffer() = 0;
    virtual void popFramebuffer() = 0;
    virtual void releaseFramebuffer(int framebuffer_id) = 0;
    
    // These methods are used by legacy stimulus classes to manage per-context OpenGL resources.
    // They should *not* be used internally by StimulusDisplay.
    int getNContexts() const { return 1; }
    OpenGLContextLock setCurrent(int i = 0 /*ignored*/) const { return setCurrentOpenGLContext(); }
    int getCurrentContextIndex() const { return 0; }
    
protected:
    const boost::shared_ptr<OpenGLContextManager> & getContextManager() const { return contextManager; }
    const boost::shared_ptr<Clock> & getClock() const { return clock; }
    void getBackgroundColor(double &red, double &green, double &blue, double &alpha) const {
        std::tie(red, green, blue, alpha) = std::tie(backgroundRed, backgroundGreen, backgroundBlue, backgroundAlpha);
    }
    bool getDisplayUpdatesStarted() const { return displayUpdatesStarted; }
    
    void setMainDisplayRefreshRate(double value) { mainDisplayRefreshRate = value; }
    void setCurrentOutputTimeUS(MWTime value) { currentOutputTimeUS = value; }
    OpenGLContextLock setCurrentOpenGLContext() const;
    
    void refreshDisplay();
    void reportSkippedFrames(double numSkippedFrames) const;
    
    virtual void prepareContext(int context_id, bool isMainContext) = 0;
    virtual void startDisplayUpdates() = 0;
    virtual void stopDisplayUpdates() = 0;
    virtual void renderDisplay(bool needDraw, const std::vector<boost::shared_ptr<Stimulus>> &stimsToDraw) = 0;
    
    std::condition_variable refreshCond;
    using unique_lock = std::unique_lock<std::mutex>;
    using lock_guard = std::lock_guard<unique_lock::mutex_type>;
    lock_guard::mutex_type mutex;
    
private:
    void stateSystemCallback(const Datum &data, MWorksTime time);
    void ensureRefresh(unique_lock &lock);
    
    const boost::shared_ptr<OpenGLContextManager> contextManager;
    const boost::shared_ptr<Clock> clock;
    
    const bool useColorManagement;
    double backgroundRed, backgroundGreen, backgroundBlue, backgroundAlpha;  // background color
    bool redrawOnEveryRefresh;
    bool announceStimuliOnImplicitUpdates;
    
    double boundsLeft, boundsRight, boundsTop, boundsBottom;  // display bounds
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
    
    bool waitingForRefresh;
    bool needDraw;
    bool paused;
    bool didDrawWhilePaused;
    
};


END_NAMESPACE_MW


#endif

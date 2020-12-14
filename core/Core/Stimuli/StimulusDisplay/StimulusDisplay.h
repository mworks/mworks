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
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>

#include "Clock.h"
#include "LinkedList.h"
#include "OpenGLContextLock.h"


BEGIN_NAMESPACE_MW


// forward declarations
class Datum;
class Stimulus;
class StimulusNode;
class OpenGLContextManager;
class VariableCallbackNotification;


class StimulusDisplay : public boost::enable_shared_from_this<StimulusDisplay>, boost::noncopyable {
    
protected:
    const boost::shared_ptr<OpenGLContextManager> opengl_context_manager;
    const boost::shared_ptr<Clock> clock;
    
    std::vector<int> context_ids;
    int current_context_index;
    boost::shared_ptr<LinkedList<StimulusNode>> display_stack;
    
    boost::mutex display_lock;
    using unique_lock = boost::mutex::scoped_lock;
    boost::condition_variable refreshCond;
    bool waitingForRefresh;
    
    bool needDraw;
    bool redrawOnEveryRefresh;
    
    double left, right, top, bottom; // display bounds
    GLKMatrix4 projectionMatrix;
    double backgroundRed, backgroundGreen, backgroundBlue, backgroundAlpha;  // background color
    
    boost::shared_ptr<VariableCallbackNotification> stateSystemNotification;
    std::atomic_bool displayUpdatesStarted;
    double mainDisplayRefreshRate;
    std::atomic<MWTime> currentOutputTimeUS;
    
    bool paused;
    bool didDrawWhilePaused;
    
    bool announceStimuliOnImplicitUpdates;
    std::vector<Datum> stimAnnouncements;
    
    const bool useColorManagement;
    int framebuffer_id;
    
    virtual void prepareContext(int contextIndex) = 0;
    virtual void setMainDisplayRefreshRate() = 0;
    
    void setDisplayBounds();
    void refreshMainDisplay();
    void refreshMirrorDisplay(int contextIndex) const;
    void drawDisplayStack();
    void ensureRefresh(unique_lock &lock);
    
    void announceDisplayUpdate(bool updateIsExplicit);
    
    void reportSkippedFrames(double numSkippedFrames) const;
    
    void stateSystemCallback(const Datum &data, MWorksTime time);
    virtual void startDisplayUpdates() = 0;
    virtual void stopDisplayUpdates() = 0;
    
public:
    static void getDisplayBounds(const Datum &mainScreenInfo,
                                 double &left,
                                 double &right,
                                 double &bottom,
                                 double &top);
    
    explicit StimulusDisplay(bool useColorManagement);
    virtual ~StimulusDisplay();
    
    void addContext(int _context_id);
    
    // These are meant to be used by stimulus classes that need to manage per-context GL resources.
    // They should *not* be used internally by StimulusDisplay.
    int getNContexts() { return (context_ids.empty() ? 0 : 1); }
    OpenGLContextLock setCurrent(int i);
    int getCurrentContextIndex() { return current_context_index; }
    
    void addStimulusNode(const boost::shared_ptr<StimulusNode> &stimnode);
    
    void setBackgroundColor(double red, double green, double blue, double alpha);
    void setRedrawOnEveryRefresh(bool redrawOnEveryRefresh);
    void setAnnounceStimuliOnImplicitUpdates(bool announceStimuliOnImplicitUpdates);
    bool getUseColorManagement() const { return useColorManagement; }
    MWTime updateDisplay();
    void clearDisplay();
    void getDisplayBounds(double &left, double &right, double &bottom, double &top);
    const GLKMatrix4& getProjectionMatrix() const { return projectionMatrix; }
    double getMainDisplayRefreshRate() const { return mainDisplayRefreshRate; }
    MWTime getCurrentOutputTimeUS() const { return currentOutputTimeUS; }
    
    static boost::shared_ptr<StimulusDisplay> createPlatformStimulusDisplay(bool useColorManagement);
    static boost::shared_ptr<StimulusDisplay> getCurrentStimulusDisplay();
    
};


END_NAMESPACE_MW


#endif

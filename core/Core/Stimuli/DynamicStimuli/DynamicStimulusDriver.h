/*
 *  DynamicStimulusDriver.h
 *  DriftingGratingStimulusPlugin
 *
 *  Created by bkennedy on 11/14/08.
 *  Copyright 2008 mit. All rights reserved.
 *
 */

#ifndef DYNAMIC_STIMULUS_H
#define DYNAMIC_STIMULUS_H

#include "StimulusDisplay.h"
#include "VariableNotification.h"


BEGIN_NAMESPACE_MW


class DynamicStimulusDriver {
	
public:
	DynamicStimulusDriver();
    
    virtual ~DynamicStimulusDriver();
	
    void play();
    void stop();
    
    void pause();
    void unpause();

protected:
    static constexpr MWTime NOT_STARTED = -1LL;
    static constexpr MWTime NOT_PAUSED = -1LL;
    
    bool isPlaying() const { return (startTime != NOT_STARTED); }
    bool isPaused() const { return (pauseTime != NOT_PAUSED); }
    MWTime getStartTime() const { return startTime; }
    MWTime getCurrentTime() const { return StimulusDisplay::getDefaultStimulusDisplay()->getCurrentOutputTimeUS(); }
    MWTime getElapsedTime() const;
    
    virtual void startPlaying();
    virtual void stopPlaying();
    
    virtual void beginPause();
    virtual void endPause();
    
	boost::mutex stim_lock;
	
private:
    void stateSystemCallback(const Datum &data, MWorksTime time);
    
	MWTime startTime;
	MWTime pauseTime;
	MWTime elapsedTimeWhilePaused;
    
    boost::shared_ptr<VariableCallbackNotification> stateSystemCallbackNotification;

};


typedef boost::shared_ptr<DynamicStimulusDriver> DynamicStimulusDriverPtr;


END_NAMESPACE_MW


#endif




























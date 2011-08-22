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

#include "StandardStimuli.h"
#include "VariableNotification.h"

#include <boost/noncopyable.hpp>


BEGIN_NAMESPACE_MW


class DynamicStimulusDriver : boost::noncopyable {
	
public:
	DynamicStimulusDriver();
    
    virtual ~DynamicStimulusDriver();
	
	virtual void play();
	virtual void stop();

    void stateSystemCallback(const Datum &data, MWorksTime time);
    
protected:
    static const MWTime NOT_STARTED = -1LL;
    
    bool isPlaying() const { return playing; }
    MWTime getStartTime() const { return startTime; }
    MWTime getElapsedTime();
    
	boost::mutex stim_lock;
	
private:
    bool playing;
	MWTime startTime;
    
    boost::shared_ptr<VariableCallbackNotification> stateSystemCallbackNotification;

};


typedef boost::shared_ptr<DynamicStimulusDriver> DynamicStimulusDriverPtr;


END_NAMESPACE_MW


#endif




























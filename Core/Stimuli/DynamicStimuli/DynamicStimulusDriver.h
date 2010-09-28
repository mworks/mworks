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

namespace mw {

class DynamicStimulusDriver {
	
protected: 
	boost::shared_ptr<Clock> clock;
	boost::shared_ptr<Variable> frames_per_second;
	
    shared_ptr<VariableCallbackNotification> state_system_callback;
	
	bool started;
	MWTime start_time;
	MWTime interval_us;
    
	boost::mutex stim_lock;
	
public:
	
	DynamicStimulusDriver(shared_ptr<Scheduler> scheduler, shared_ptr<Variable> frames_per_second);
	
	DynamicStimulusDriver(const DynamicStimulusDriver &tocopy);
    
    virtual ~DynamicStimulusDriver();
	
	virtual void play();
	virtual void stop();

	virtual void willPlay() { }
	virtual void didStop() { }

	virtual MWTime getElapsedTime();
    virtual int getFrameNumber();
    
    void stateSystemCallback(const Datum& data, MWorksTime time);
};

}

#endif /* DYNAMIC_STIMULUS_H */



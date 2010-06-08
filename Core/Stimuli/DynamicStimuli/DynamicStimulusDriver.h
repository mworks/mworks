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
#include "boost/enable_shared_from_this.hpp"
#include "VariableNotification.h"

namespace mw{

class DynamicStimulusDriver : public boost::enable_shared_from_this<DynamicStimulusDriver>, public VariableNotification {
	
protected: 
	boost::shared_ptr<Scheduler> scheduler;
	boost::shared_ptr<Clock> clock;
    boost::shared_ptr<StimulusDisplay> display;
	
	boost::shared_ptr<ScheduleTask> schedule_node;
	
	boost::shared_ptr<Variable> frames_per_second;
	boost::shared_ptr<Variable> statistics_reporting;
	boost::shared_ptr<Variable> error_reporting;
	
    shared_ptr<VariableCallbackNotification> state_system_callback;
	
	bool started;
	MWTime start_time;
	MWTime interval_us;
    
	boost::mutex stim_lock;
	
public:
	
	DynamicStimulusDriver(const boost::shared_ptr<Scheduler> &a_scheduler,
					 const boost::shared_ptr<StimulusDisplay> &a_display,
					 const boost::shared_ptr<Variable> &frames_per_second_var,
					 const boost::shared_ptr<Variable> &stats_var,
					 const boost::shared_ptr<Variable> &error_reporting_var);
	
	DynamicStimulusDriver(const DynamicStimulusDriver &tocopy);
    
    virtual ~DynamicStimulusDriver();
	
	virtual void play();
	virtual void stop();

	virtual MWTime getElapsedTime();
    virtual int getFrameNumber();
    
	virtual void callUpdateDisplay();
	virtual Datum getCurrentAnnounceDrawData();
    
    void stateSystemCallback(const Datum& data, MWorksTime time);
};

void *nextUpdate(const shared_ptr<DynamicStimulusDriver> &ds);
}

#endif /* DYNAMIC_STIMULUS_H */



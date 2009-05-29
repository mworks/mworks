/*
 *  Dynamicstimulus.h
 *  DriftingGratingStimulusPlugin
 *
 *  Created by bkennedy on 11/14/08.
 *  Copyright 2008 mit. All rights reserved.
 *
 */

#ifndef DYNAMIC_STIMULUS_H
#define DYNAMIC_STIMULUS_H

#include "MonkeyWorksCore/StandardStimuli.h"
#include "boost/enable_shared_from_this.hpp"
using namespace mw;

class mDynamicStimulus : public Stimulus, public boost::enable_shared_from_this<mDynamicStimulus> {
	
protected: 
	boost::shared_ptr<Scheduler> scheduler;
	boost::shared_ptr<StimulusDisplay> display;
	
	boost::shared_ptr<ScheduleTask> schedule_node;
	
	boost::shared_ptr<Variable> frames_per_second;
	boost::shared_ptr<Variable> statistics_reporting;
	boost::shared_ptr<Variable> error_reporting;
	
	std::vector<boost::shared_ptr<std::vector<MonkeyWorksTime> > > times_shown;
	
	bool started;
	MonkeyWorksTime start_time;
	
	boost::mutex stim_lock;
	
public:
	
	mDynamicStimulus(const std::string &new_tag,
					 const boost::shared_ptr<Scheduler> &a_scheduler,
					 const boost::shared_ptr<StimulusDisplay> &a_display,
					 const boost::shared_ptr<Variable> &frames_per_second_var,
					 const boost::shared_ptr<Variable> &stats_var,
					 const boost::shared_ptr<Variable> &error_reporting_var);
	
	mDynamicStimulus(const mDynamicStimulus &tocopy);
	virtual Stimulus *frozenClone() = 0;
	void draw(StimulusDisplay *display) = 0;
	
	virtual void play();
	virtual void stop();
	
	virtual void callUpdateDisplay();
	virtual Data getCurrentAnnounceDrawData();
};

void *nextUpdate(const shared_ptr<mDynamicStimulus> &ds);


#endif /* DYNAMIC_STIMULUS_H */



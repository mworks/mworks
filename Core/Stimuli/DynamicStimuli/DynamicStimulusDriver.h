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

namespace mw{

class DynamicStimulusDriver : public boost::enable_shared_from_this<DynamicStimulusDriver> {
	
protected: 
	boost::shared_ptr<Scheduler> scheduler;
	boost::shared_ptr<Clock> clock;
    boost::shared_ptr<StimulusDisplay> display;
	
	boost::shared_ptr<Variable> frames_per_second;
	
	bool started;
	MWTime start_time;
	MWTime interval_us;
    
	boost::mutex stim_lock;
	
public:
	
	DynamicStimulusDriver(const boost::shared_ptr<Scheduler> &a_scheduler,
					 const boost::shared_ptr<StimulusDisplay> &a_display,
					 const boost::shared_ptr<Variable> &frames_per_second_var);
	
	DynamicStimulusDriver(const DynamicStimulusDriver &tocopy);
    
    virtual ~DynamicStimulusDriver();
	
	virtual void play();
	virtual void stop();

	virtual void willPlay() { }
	virtual void didStop() { }

	virtual MWTime getElapsedTime();
    virtual int getFrameNumber();
    
	virtual Datum getCurrentAnnounceDrawData();
};

}

#endif /* DYNAMIC_STIMULUS_H */



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
    
    bool isPlaying() const { return (startTime != NOT_STARTED); }
    MWTime getStartTime() const { return startTime; }
    MWTime getCurrentTime() const { return StimulusDisplay::getCurrentStimulusDisplay()->getCurrentOutputTimeUS(); }
    MWTime getElapsedTime() const;
    
    virtual void startPlaying();
    virtual void stopPlaying();
    
	boost::mutex stim_lock;
	
private:
	MWTime startTime;
    boost::shared_ptr<VariableCallbackNotification> stateSystemCallbackNotification;

};


typedef boost::shared_ptr<DynamicStimulusDriver> DynamicStimulusDriverPtr;


END_NAMESPACE_MW


#endif




























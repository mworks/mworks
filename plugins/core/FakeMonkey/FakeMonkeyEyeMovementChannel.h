/*
 *  FakeMonkeyMovementChannel.h
 *  FakeMonkey
 *
 *  Created by bkennedy on 10/24/08.
 *  Copyright 2008 mit. All rights reserved.
 *
 */

#ifndef FAKE_MONKEY_EYE_MOVEMENT_CHANNEL_H
#define FAKE_MONKEY_EYE_MOVEMENT_CHANNEL_H

#include "FakeMonkeyStatus.h"
#include "MWorksCore/Clock.h"
#include "MWorksCore/GenericVariable.h"
using namespace mw;

class mFakeMonkeyEyeMovementChannel : public mw::Component {
protected:
	boost::shared_ptr <Variable> eye_h;
	boost::shared_ptr <Variable> eye_v;

	
	MWorksTime update_period;
	int samples_per_update;
	shared_ptr <Clock> clock;
	shared_ptr <boost::mutex> monkey_lock;
	shared_ptr <mFakeMonkeyStatus> status;
	shared_ptr <MWorksTime> saccade_duration;
	shared_ptr <MWorksTime> saccade_start_time;
	shared_ptr <float> saccade_start_h;
	shared_ptr <float> saccade_target_h;
	shared_ptr <float> saccade_start_v;
	shared_ptr <float> saccade_target_v;
public:
	mFakeMonkeyEyeMovementChannel(const shared_ptr<Variable> &eye_h_variable,
								  const shared_ptr<Variable> &eye_v_variable,
								  const MWorksTime update_period_,
								  const int samples_per_update_);
	
	MWorksTime getUpdatePeriod() const;
	
	virtual void setChannelParams(const shared_ptr <Clock> &a_clock,
								  const shared_ptr <boost::mutex> monkey_lock_,
								  const shared_ptr <mFakeMonkeyStatus> &status,
								  const shared_ptr <MWorksTime> &saccade_start_time,
								  const shared_ptr <MWorksTime> &saccade_duration,
								  const shared_ptr <float> &saccade_start_h,
								  const shared_ptr <float> &saccade_start_v,
								  const shared_ptr <float> &saccade_target_h,
								  const shared_ptr <float> &saccade_target_v);
	
	void update();
};

#endif

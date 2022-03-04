/*
 *  FakeMonkeyEyeMovementChannel.cpp
 *  FakeMonkey
 *
 *  Created by bkennedy on 10/24/08.
 *  Copyright 2008 mit. All rights reserved.
 *
 */

#include "FakeMonkeyEyeMovementChannel.h"

#include <algorithm>

using namespace mw;


mFakeMonkeyEyeMovementChannel::mFakeMonkeyEyeMovementChannel(const shared_ptr<Variable> &eye_h_variable,
															 const shared_ptr<Variable> &eye_v_variable,
															 const MWorksTime update_period_,
															 const int samples_per_update_) {
	eye_h = eye_h_variable;
	eye_v = eye_v_variable;
	update_period = update_period_;	
	samples_per_update = samples_per_update_;
}

MWorksTime mFakeMonkeyEyeMovementChannel::getUpdatePeriod() const {
	return update_period;
}

void mFakeMonkeyEyeMovementChannel::update() {
	boost::mutex::scoped_lock lock(*monkey_lock);
	MWorksTime start_time = clock->getCurrentTimeUS();

	for(int current_sample = 0; current_sample < samples_per_update; ++current_sample) {
		MWorksTime current_time = start_time+((current_sample*update_period)/samples_per_update);

		double val_h = 0;
		double val_v = 0;
		if(*status == M_FAKE_MONKEY_SACCADING){
			double percent_done = (double)(current_time - *saccade_start_time) / (double)(*saccade_duration);
			percent_done = std::max(std::min(1.0, percent_done),0.0);
			val_h = (1.0-percent_done) * (*saccade_start_h) + percent_done * (*saccade_target_h) + 0.05*(double)((double)rand()/(double)RAND_MAX);				
			val_v = (1.0-percent_done) * (*saccade_start_v) + percent_done * (*saccade_target_v) + 0.05*(double)((double)rand()/(double)RAND_MAX);				
		} else {
			// fixating, hang out at the target
			val_h = (*saccade_target_h) + 0.05*(double)((double)rand()/(double)RAND_MAX);
			val_v = (*saccade_target_v) + 0.05*(double)((double)rand()/(double)RAND_MAX);
		}
		eye_h->setValue(std::max(std::min(val_h, 90.0), -90.0), current_time);
		eye_v->setValue(std::max(std::min(val_v, 90.0), -90.0), current_time);
	}
}

void mFakeMonkeyEyeMovementChannel::setChannelParams(const shared_ptr <Clock> &a_clock,
													 const shared_ptr <boost::mutex> monkey_lock_,
													 const shared_ptr <mFakeMonkeyStatus> &status_,
													 const shared_ptr <MWorksTime> &saccade_start_time_,
													 const shared_ptr <MWorksTime> &saccade_duration_,
													 const shared_ptr <float> &saccade_start_h_,
													 const shared_ptr <float> &saccade_start_v_,
													 const shared_ptr <float> &saccade_target_h_,
													 const shared_ptr <float> &saccade_target_v_) {
	clock = a_clock;
	monkey_lock = monkey_lock_;
	status = status_;
	saccade_start_time = saccade_start_time_;
	saccade_duration = saccade_duration_;
	saccade_start_h = saccade_start_h_;
	saccade_target_h = saccade_target_h_;
	saccade_start_v = saccade_start_v_;
	saccade_target_v = saccade_target_v_;
}





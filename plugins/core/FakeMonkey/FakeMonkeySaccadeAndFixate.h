/*
 *  FakeMonkeySaccadeAndFixate.h
 *  FakeMonkey
 *
 *  Created by labuser on 10/8/08.
 *  Copyright 2008 MIT. All rights reserved.
 *
 */

#ifndef FAKE_MONKEY_SACCADE_AND_FIXATE_H
#define FAKE_MONKEY_SACCADE_AND_FIXATE_H

#include "MWorksCore/TrialBuildingBlocks.h"
#include "FakeMonkey.h"

class mFakeMonkeySaccadeAndFixate : public Action {
protected:
	shared_ptr<mFakeMonkey> monkey;
	shared_ptr<Variable> h_loc; 
	shared_ptr<Variable> v_loc;
	shared_ptr<Variable> duration;
public:
	mFakeMonkeySaccadeAndFixate(const shared_ptr<mFakeMonkey> &_monkey, 
								const shared_ptr<Variable> &_h_loc,
								const shared_ptr<Variable> &_v_loc,
								const shared_ptr<Variable> &_duration);
	
	virtual ~mFakeMonkeySaccadeAndFixate();
	virtual bool execute();
};

#endif


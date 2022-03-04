/*
 *  FakeMonkeyFixate.h
 *  FakeMonkey
 *
 *  Created by bkennedy on 10/8/08.
 *  Copyright 2008 mit. All rights reserved.
 *
 */

#ifndef FAKE_MONKEY_FIXATE_H
#define FAKE_MONKEY_FIXATE_H

#include "MWorksCore/TrialBuildingBlocks.h"
#include "FakeMonkey.h"

class mFakeMonkeyFixate : public Action {
protected:
	shared_ptr<mFakeMonkey> monkey;
	shared_ptr<Variable> duration; 
public:
	mFakeMonkeyFixate(const shared_ptr<mFakeMonkey> &_monkey, 
					  const shared_ptr<Variable> &_duration);
	
	virtual ~mFakeMonkeyFixate();
	virtual bool execute();
};




#endif
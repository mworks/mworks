/*
 *  FakeMonkeySpikeChannel.h
 *  FakeMonkey
 *
 *  Created by bkennedy on 10/24/08.
 *  Copyright 2008 mit. All rights reserved.
 *
 */

#ifndef FAKE_MONKEY_SPIKE_CHANNEL_H
#define FAKE_MONKEY_SPIKE_CHANNEL_H

#include "FakeMonkeyChannel.h"

class mFakeMonkeySpikeChannel : public mFakeMonkeyChannel {
public:
	mFakeMonkeySpikeChannel(const shared_ptr<Variable> &spike_variable_);
};

#endif

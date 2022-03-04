/*
 *  FakeMonkeyJuiceChannel.h
 *  FakeMonkey
 *
 *  Created by bkennedy on 10/24/08.
 *  Copyright 2008 mit. All rights reserved.
 *
 */

#ifndef FAKE_MONKEY_JUICE_CHANNEL_H
#define FAKE_MONKEY_JUICE_CHANNEL_H

#include "FakeMonkeyChannel.h"

class mFakeMonkeyJuiceChannel : public mFakeMonkeyChannel {
public:
	mFakeMonkeyJuiceChannel(const shared_ptr<Variable> &reward_variables_);
};

#endif

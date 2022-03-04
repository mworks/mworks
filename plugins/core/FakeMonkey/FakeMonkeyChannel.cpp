/*
 *  FakeMonkeyChannel.cpp
 *  FakeMonkey
 *
 *  Created by bkennedy on 10/24/08.
 *  Copyright 2008 mit. All rights reserved.
 *
 */

#include "FakeMonkeyChannel.h"
using namespace mw;

mFakeMonkeyChannel::mFakeMonkeyChannel(const shared_ptr<Variable> &variable_) {
	variable = variable_;
}

boost::shared_ptr <Variable> mFakeMonkeyChannel::getVariable() {
	return variable;
}



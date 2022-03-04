/*
 *  FakeMonkeyChannel.h
 *  FakeMonkey
 *
 *  Created by bkennedy on 10/24/08.
 *  Copyright 2008 mit. All rights reserved.
 *
 */

#ifndef FAKE_MONKEY_CHANNEL_H
#define FAKE_MONKEY_CHANNEL_H

#include "MWorksCore/GenericVariable.h"
using namespace mw;

class mFakeMonkeyChannel : public mw::Component {
protected:
	boost::shared_ptr <Variable> variable;
public:
	mFakeMonkeyChannel(const shared_ptr<Variable> &variable_);
	virtual boost::shared_ptr <Variable> getVariable();
};

#endif


/*
 *  FakeMonkeyJuiceNotification.h
 *  FakeMonkey
 *
 *  Created by labuser on 10/8/08.
 *  Copyright 2008 MIT. All rights reserved.
 *
 */

#ifndef FAKE_MONKEY_JUICE_NOTIFICATION_H
#define FAKE_MONKEY_JUICE_NOTIFICATION_H

#include "MWorksCore/VariableNotification.h"
using namespace mw;

class mFakeMonkeyJuiceNotification : public VariableNotification {
public:
	virtual void notify(const Datum& data, MWorksTime timeUS);	
};

#endif


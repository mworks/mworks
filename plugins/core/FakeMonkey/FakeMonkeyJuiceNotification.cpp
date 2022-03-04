/*
 *  FakeMonkeyJuiceNotification.cpp
 *  FakeMonkey
 *
 *  Created by labuser on 10/8/08.
 *  Copyright 2008 MIT. All rights reserved.
 *
 */

#include "FakeMonkeyJuiceNotification.h"

void mFakeMonkeyJuiceNotification::notify(const Datum& data, MWorksTime timeUS) {
	mprintf("YUM! Fake monkey loves his juice!");
}

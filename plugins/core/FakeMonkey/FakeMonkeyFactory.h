/*
 *  FakeMonkeyFactory.h
 *  FakeMonkeyPlugin
 *
 *  Created by labuser on 8/18/08.
 *  Copyright 2008 MIT. All rights reserved.
 *
 */

#ifndef FAKE_MONKEY_FACTORY_H
#define FAKE_MONKEY_FACTORY_H

#include "MWorksCore/ComponentFactory.h"
using namespace mw;

class mFakeMonkeyFactory : public ComponentFactory {
	virtual shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
												ComponentRegistry *reg);
};

#endif


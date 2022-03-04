/*
 *  FakeMonkeySaccadeAndFixateFactory.h
 *  FakeMonkey
 *
 *  Created by labuser on 10/8/08.
 *  Copyright 2008 mit. All rights reserved.
 *
 */

#ifndef FAKE_MONKEY_SACCADE_AND_FIXATE_FACTORY_H
#define FAKE_MONKEY_SACCADE_AND_FIXATE_FACTORY_H

#include "MWorksCore/ComponentFactory.h"
using namespace mw;

class mFakeMonkeySaccadeAndFixateFactory : public ComponentFactory{
	virtual shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
												ComponentRegistry *reg);
};

#endif
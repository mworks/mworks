/*
 *  ZenStateSystemFactory.cpp
 *  MWorksCore
 *
 *  Created by bkennedy on 10/29/08.
 *  Copyright 2008 mit. All rights reserved.
 *
 */

#include "ZenStateSystemFactory.h"
#include "ZenStateSystem.h"


BEGIN_NAMESPACE_MW


shared_ptr<mw::Component> StandardStateSystemFactory::createObject(std::map<std::string, std::string> parameters,
																 ComponentRegistry *reg){
	shared_ptr<Clock> a_clock = Clock::instance();
	return shared_ptr<mw::Component>(new StandardStateSystem(a_clock));
}
    

END_NAMESPACE_MW

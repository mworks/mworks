/*
 *  ZenStateSystemFactory.h
 *  MonkeyWorksCore
 *
 *  Created by bkennedy on 10/29/08.
 *  Copyright 2008 mit. All rights reserved.
 *
 */

#ifndef ZEN_STATE_SYSTEM_FACTORY_H
#define ZEN_STATE_SYSTEM_FACTORY_H

#include "MonkeyWorksCore/ComponentFactory.h"
#include "MonkeyWorksCore/ComponentRegistry_new.h"
#include "ZenStateSystem.h"

class StandardStateSystemFactory : public ComponentFactory {
public:
	virtual shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
												mwComponentRegistry *reg);
};

#endif


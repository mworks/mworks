/*
 *  ZenStateSystemFactory.h
 *  MWorksCore
 *
 *  Created by bkennedy on 10/29/08.
 *  Copyright 2008 mit. All rights reserved.
 *
 */

#ifndef ZEN_STATE_SYSTEM_FACTORY_H
#define ZEN_STATE_SYSTEM_FACTORY_H

#include "MWorksCore/ComponentFactory.h"
#include "MWorksCore/ComponentRegistry.h"
#include "ZenStateSystem.h"


BEGIN_NAMESPACE_MW


class StandardStateSystemFactory : public ComponentFactory {
public:
	virtual shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
												ComponentRegistry *reg);
};


END_NAMESPACE_MW


#endif


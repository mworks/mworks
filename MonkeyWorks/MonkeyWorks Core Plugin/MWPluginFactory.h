/*
 *  ÇPROJECTNAMEÈFactory.h
 *  MWPlugin
 *
 *  Created by David Cox on 2/26/09.
 *  Copyright 2009 Harvard University. All rights reserved.
 *
 */


#ifndef ÇPROJECTNAMEÈ_FACTORY_H
#define ÇPROJECTNAMEÈ_FACTORY_H

#include "MonkeyWorksCore/ComponentFactory.h"
using namespace mw;

class ÇPROJECTNAMEÈFactory : public ComponentFactory {
	virtual shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
                                                   mw::ComponentRegistry *reg);
};

#endif

/*
 *  ÇPROJECTNAMEÈFactory.h
 *  MWPlugin
 *
 *  Created by ÇFULLUSERNAMEÈ on ÇDATEÈ.
 *  Copyright ÇYEARÈ ÇORGANIZATIONNAMEÈ. All rights reserved.
 *
 */


#ifndef ÇPROJECTNAMEÈ_FACTORY_H
#define ÇPROJECTNAMEÈ_FACTORY_H

#include "ÇPROJECTNAMEÈ.h"

#include "MonkeyWorksCore/ComponentFactory.h"
using namespace mw;

class ÇPROJECTNAMEÈFactory : public ComponentFactory {
	virtual shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
                                                   mw::ComponentRegistry *reg);
};

#endif

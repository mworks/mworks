/*
 *  DriftingGratingStimulusFactory.h
 *  MWorksCore
 *
 *  Created by bkennedy on 8/26/08.
 *  Copyright 2008 MIT. All rights reserved.
 *
 */


#ifndef DRIFTING_GRATNG_STIMULUS_FACTORY_H
#define DRIFTING_GRATNG_STIMULUS_FACTORY_H

#include "MWorksCore/ComponentFactory.h"
using namespace mw;

class DriftingGratingStimulusFactory : public ComponentFactory {
	virtual shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
                                                   mw::ComponentRegistry *reg);
};

#endif

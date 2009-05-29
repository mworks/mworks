/*
 *  DriftingGratingStimulusFactory.h
 *  MonkeyWorksCore
 *
 *  Created by bkennedy on 8/26/08.
 *  Copyright 2008 MIT. All rights reserved.
 *
 */


#ifndef DRIFTING_GRATNG_STIMULUS_FACTORY_H
#define DRIFTING_GRATNG_STIMULUS_FACTORY_H

#include "MonkeyWorksCore/ComponentFactory.h"
using namespace mw;

class mDriftingGratingStimulusFactory : public ComponentFactory {
	virtual shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
												mwComponentRegistry *reg);
};

#endif

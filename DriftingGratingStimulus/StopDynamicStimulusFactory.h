/*
 *  StopDynamicStimulusFactory.h
 *  DriftingGratingStimulusPlugin
 *
 *  Created by bkennedy on 11/14/08.
 *  Copyright 2008 mit. All rights reserved.
 *
 */


#ifndef STOP_DYNAMIC_STIMULUS_FACTORY_H
#define STOP_DYNAMIC_STIMULUS_FACTORY_H

#include "MonkeyWorksCore/ComponentFactory.h"
using namespace mw;

class mStopDynamicStimulusFactory : public ComponentFactory {
	virtual boost::shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
													   mwComponentRegistry *reg);
};

#endif

/*
 *  PlayDynamicStimulusFactory.h
 *  DriftingGratingStimulusPlugin
 *
 *  Created by bkennedy on 11/14/08.
 *  Copyright 2008 mit. All rights reserved.
 *
 */

#ifndef DYNAMIC_STIMULUS_ACTION_FACTORY_H
#define DYNAMIC_STIMULUS_ACTION_FACTORY_H

#include "ComponentFactory.h"
namespace mw{

class PlayDynamicStimulusFactory : public ComponentFactory {
	virtual boost::shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
                                                          mw::ComponentRegistry *reg);
};

class StopDynamicStimulusFactory : public ComponentFactory {
	virtual boost::shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
                                                          mw::ComponentRegistry *reg);
};
}

#endif

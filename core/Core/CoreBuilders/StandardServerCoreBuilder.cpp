/**
 * StandardServerCoreBuilder.cpp
 *
 * History:
 * paul on 1/20/06 - Created.
 *
 * Copyright 2006 MIT. All rights reserved.
 */

#include "StandardServerCoreBuilder.h"

#include "ComponentRegistry.h"

#import "Utilities.h"
#import "Experiment.h"
#import "Scheduler.h"
#import "StateSystem.h"
#import "Event.h"
#import "PluginServices.h"
#import "TrialBuildingBlocks.h"
#import "VariableRegistry.h"
#import "StandardVariables.h"
#import "EventBuffer.h"
#import "OpenALContextManager.h"
#include "PlatformDependentServices.h"


BEGIN_NAMESPACE_MW


StandardServerCoreBuilder::StandardServerCoreBuilder() { }
StandardServerCoreBuilder::~StandardServerCoreBuilder() { }

/****************************************************************
 *             AbstractCoreBuilder  Methods
 ***************************************************************/
bool StandardServerCoreBuilder::buildProcessWillStart() { return true; }

bool StandardServerCoreBuilder::initializeEventBuffers() {
    initEventBuffers();
    return true;
}

bool StandardServerCoreBuilder::initializeRegistries() {
	initializeServiceRegistries();
	
	return true;
}

bool StandardServerCoreBuilder::initializeGlobalParameters() {
    initializeStandardVariables(global_variable_registry);
	try {
		loadSetupVariables();
	} catch (std::exception& e){
		mwarning(M_PARSER_MESSAGE_DOMAIN, "Unable to load setup variables.  Error was: %s", e.what());
	}
	GlobalMessageOrigin = M_SERVER_MESSAGE_ORIGIN;
   
    return true;
}

bool StandardServerCoreBuilder::loadPlugins() {
	
	shared_ptr<ComponentRegistry> component_registry = 
									ComponentRegistry::getSharedRegistry();
	
	readPlugins(pluginPath());
    return true;
}


static shared_ptr<mw::Component> createRealtimeComponent(const shared_ptr<ComponentRegistry> &componentRegistry,
                                                         const Datum &realtimeComponentsValue,
                                                         const std::string &componentType,
                                                         const std::string &defaultPluginName)
{
    std::string pluginName(defaultPluginName);
    
    if (realtimeComponentsValue.isDictionary()) {
        Datum componentValue = realtimeComponentsValue.getElement(componentType);
        if (componentValue.isString()) {
            pluginName = componentValue.getString();
        }
    }
    
    mprintf(M_SYSTEM_MESSAGE_DOMAIN, "  %s:\t%s", componentType.c_str(), pluginName.c_str());
    
    return componentRegistry->createNewObject(pluginName, map<string, string>());
}


bool StandardServerCoreBuilder::chooseRealtimeComponents() {
    shared_ptr<ComponentRegistry> componentRegistry = ComponentRegistry::getSharedRegistry();
    Datum realtimeComponentsValue = realtimeComponents->getValue();
    
    mprintf(M_SYSTEM_MESSAGE_DOMAIN, "Creating realtime components:");

	Clock::registerInstance(createRealtimeComponent(componentRegistry, realtimeComponentsValue, M_REALTIME_CLOCK_KEY, "MachClock"));
	Scheduler::registerInstance(createRealtimeComponent(componentRegistry, realtimeComponentsValue, M_REALTIME_SCHEDULER_KEY, "ZenScheduler"));
	StateSystem::registerInstance(createRealtimeComponent(componentRegistry, realtimeComponentsValue, M_REALTIME_STATE_SYSTEM_KEY, "ZenStateSystem"));

    return true;
}

bool StandardServerCoreBuilder::startInterpreters() {
   // GlobalPerlInterpreter = new PerlInterpreter();
  //  GlobalPerlInterpreter->evalString(
  //                            "mprintf(qq(Perl interpreter is online\\n));");
    return true;
}

bool StandardServerCoreBuilder::startRealtimeServices() {
	shared_ptr <Clock> clock = Clock::instance();
	clock->startClock();
    return true;
}

bool StandardServerCoreBuilder::customInitialization() { 
    // ignore sigpipes
    signal(SIGPIPE, SIG_IGN);
	
    return true; 
}

bool StandardServerCoreBuilder::buildProcessWillEnd() { return true; }


END_NAMESPACE_MW

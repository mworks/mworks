/**
 * StandardClientCoreBuilder.cpp
 *
 * History:
 * paul on 2/7/06 - Created.
 *
 * Copyright 2006 MIT. All rights reserved.
 */

#include "StandardClientCoreBuilder.h"

#import "Event.h"
#import "EventBuffer.h"
#import "PluginServices.h"
#import "ScarabServices.h"
#import "StandardVariables.h"
#include "PlatformDependentServices.h"


BEGIN_NAMESPACE_MW


StandardClientCoreBuilder::StandardClientCoreBuilder() { }

StandardClientCoreBuilder::~StandardClientCoreBuilder() { }

/****************************************************************
 *             AbstractCoreBuilder  Methods
 ***************************************************************/
bool StandardClientCoreBuilder::buildProcessWillStart() { return true; }

bool StandardClientCoreBuilder::initializeEventBuffers() {
    initEventBuffers();
    return true;
}

bool StandardClientCoreBuilder::initializeRegistries() {
	initializeServiceRegistries();
	return true;
}

bool StandardClientCoreBuilder::initializeGlobalParameters() {
    initializeStandardVariables(global_variable_registry);
	GlobalMessageOrigin = M_CLIENT_MESSAGE_ORIGIN;
	return true;
}

bool StandardClientCoreBuilder::loadPlugins() {
	
    readPlugin("MachClockPlugin");
    //readPlugin("ZenSchedulerPlugin");
    readPlugin("LowPrioritySchedulerPlugin");
	//readPlugins(pluginPath());
	return true;
}

bool StandardClientCoreBuilder::chooseRealtimeComponents() {
	
	shared_ptr<ComponentRegistry> component_registry = ComponentRegistry::getSharedRegistry();
	
	shared_ptr<mw::Component> clock = component_registry->createNewObject("MachClock", map<string, string>());
	Clock::registerInstance(clock);
	
	shared_ptr<mw::Component> scheduler = component_registry->createNewObject("LowPriorityScheduler", map<string, string>());
	Scheduler::registerInstance(scheduler);
	
	
    return true;
}

bool StandardClientCoreBuilder::startInterpreters() {
    // this may not be needed on the client side because the 
    // experiment packager uses its own interpreter
    //GlobalPerlInterpreter = new PerlInterpreter();
    //GlobalPerlInterpreter->evalString(
    //                          "mprintf(qq(Perl interpreter is online\\n));");
    return true;
}

bool StandardClientCoreBuilder::startRealtimeServices() {
	shared_ptr <Clock> clock = Clock::instance();
	clock->startClock();
    return true;
}

bool StandardClientCoreBuilder::customInitialization() { 
    // ignore sigpipes
    signal(SIGPIPE, SIG_IGN);
    return true; 
}

bool StandardClientCoreBuilder::buildProcessWillEnd() { return true; }


END_NAMESPACE_MW

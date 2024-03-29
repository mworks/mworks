/*
 *  FullCoreEnvironmentTest.cpp
 *  MWorksCore
 *
 *  Created by David Cox on 9/19/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#include "FullCoreEnvironmentTest.h"
#include <sstream>
#include "MWorksCore/CorebuilderForeman.h"
#include "MWorksCore/TestBedCoreBuilder.h"
#include "MWorksCore/Experiment.h"
#include "MWorksCore/EventBuffer.h"
#include "MWorksCore/PluginServices.h"
#include "MWorksCore/StandardVariables.h"
#include "MWorksCore/DataFileManager.h"


BEGIN_NAMESPACE_MW


void FullCoreEnvironmentTestFixture::setUp(){
    TestBedCoreBuilder builder;
    CoreBuilderForeman::constructCoreStandardOrder(&builder);
}

void FullCoreEnvironmentTestFixture::tearDown(){
	// this is actually a touch tricky to do without leaking, and it 
	// probably should be handlable by the builder in some way
	/*if(GlobalCurrentExperiment){
		delete GlobalCurrentExperiment;
		GlobalCurrentExperiment = 0;
	}*/
	
	/*if(global_variable_registry){
		delete global_variable_registry;
		global_variable_registry = 0;
	}*/

  ComponentRegistry::detachSharedRegistryPtr();

	DataFileManager::destroy();

	if(registries_are_initialized) {
		registries_are_initialized = false;
	}

	StateSystem::destroy();
	Scheduler::destroy();
    Clock::instance()->stopClock();
	Clock::destroy();
}


END_NAMESPACE_MW

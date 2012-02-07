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
#include "MWorksCore/OpenALContextManager.h"
#include "MWorksCore/FilterManager.h"


BEGIN_NAMESPACE_MW


// Can't open OpenGL from here?
//#include "MWorksCore/OpenGLContextManager.h"

void FullCoreEnvironmentTestFixture::setUp(){
	builder = new TestBedCoreBuilder();
	CoreBuilderForeman::constructCoreStandardOrder(builder);
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

	if(GlobalDataFileManager) {
	  delete GlobalDataFileManager;
	  GlobalDataFileManager = 0;
	}

	OpenALContextManager::destroy();

	if(GlobalFilterManager) {
	  delete GlobalFilterManager;
	  GlobalFilterManager = 0;
	}

	

	
	if(registries_are_initialized) {
		registries_are_initialized = false;
	}

	StateSystem::destroy();
	Scheduler::destroy();
	Clock::destroy();
}


END_NAMESPACE_MW

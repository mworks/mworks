/*
 *  FullCoreEnvironmentTest.cpp
 *  MonkeyWorksCore
 *
 *  Created by David Cox on 9/19/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#include "FullCoreEnvironmentTest.h"
#include <sstream>
#include "MonkeyWorksCore/CorebuilderForeman.h"
#include "MonkeyWorksCore/TestBedCoreBuilder.h"
#include "MonkeyWorksCore/Experiment.h"
#include "MonkeyWorksCore/EventBuffer.h"
#include "MonkeyWorksCore/PluginServices.h"
#include "MonkeyWorksCore/StandardVariables.h"
#include "MonkeyWorksCore/DataFileManager.h"
#include "MonkeyWorksCore/OpenALContextManager.h"
#include "MonkeyWorksCore/FilterManager.h"
#include "MonkeyWorksCore/IODeviceManager.h"
using namespace mw;

// Can't open OpenGL from here?
//#include "MonkeyWorksCore/OpenGLContextManager.h"

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
	
	/*if(GlobalVariableRegistry){
		delete GlobalVariableRegistry;
		GlobalVariableRegistry = 0;
	}*/
	

	if(GlobalDataFileManager) {
	  delete GlobalDataFileManager;
	  GlobalDataFileManager = 0;
	}

	
	if(GlobalOpenALContextManager) {
	  delete GlobalOpenALContextManager;
	  GlobalOpenALContextManager = 0;
	}

	if(GlobalFilterManager) {
	  delete GlobalFilterManager;
	  GlobalFilterManager = 0;
	}

	if(GlobalIODeviceManager) {
	  delete GlobalIODeviceManager;
	  GlobalIODeviceManager = 0;
	}

	
//  // Can't open/close OpenGL from here?
//	if(GlobalStimMirrorController) {
//	  delete GlobalStimMirrorController;
//	  GlobalStimMirrorController = 0;
//	}
//
//	if(GlobalOpenGLContextManager) {
//	  delete GlobalOpenGLContextManager;
//	  GlobalOpenGLContextManager = 0;
//	}

	
	if(registries_are_initialized) {
		registries_are_initialized = false;
	}

	StateSystem::destroy();
	Scheduler::destroy();
	Clock::destroy();
}


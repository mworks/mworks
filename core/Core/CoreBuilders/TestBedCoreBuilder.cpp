/*
 *  TestBedCoreBuilder.cpp
 *  MWorksCore
 *
 *  Created by David Cox on 3/25/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#include "TestBedCoreBuilder.h"
#include "Utilities.h"


BEGIN_NAMESPACE_MW


bool TestBedCoreBuilder::startInterpreters(){

	mwarning(M_SYSTEM_MESSAGE_DOMAIN, "No Perl interpretter was started");
	return true;
}


END_NAMESPACE_MW

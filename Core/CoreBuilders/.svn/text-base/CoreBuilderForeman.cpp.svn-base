/**
 * CoreBuilderForeman.cpp
 *
 * History:
 * paul on 1/20/06 - Created.
 *
 * Copyright 2006 MIT. All rights reserved.
 */

#include "CoreBuilderForeman.h"
#include "AbstractCoreBuilder.h"
using namespace mw;

#define CHECK(flag) if(!flag) { return false; }

bool CoreBuilderForeman::constructCoreStandardOrder(AbstractCoreBuilder * builder) {
    CHECK(builder->buildProcessWillStart());
    CHECK(builder->initializeEventBuffers());
    CHECK(builder->initializeRegistries());
    CHECK(builder->initializeGlobalParameters());
    CHECK(builder->loadPlugins());
    CHECK(builder->chooseRealtimeComponents());
    CHECK(builder->startInterpreters());
    CHECK(builder->startRealtimeServices());
    CHECK(builder->customInitialization());
    CHECK(builder->buildProcessWillEnd());
    return true;
}
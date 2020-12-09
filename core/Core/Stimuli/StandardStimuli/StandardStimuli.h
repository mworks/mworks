/**
 * StandardStimuli.h
 *
 * Copyright 2005 MIT.  All rights reserved.
 */

#ifndef _STANDARD_STIMULI_H
#define _STANDARD_STIMULI_H

#include "ComponentRegistry.h"

#ifndef MWORKS_EXCLUDE_LEGACY_STIMULI
#include "LegacyStandardStimuli.hpp"
#endif


BEGIN_NAMESPACE_MW


void registerStandardStimuli(ComponentRegistry &registry);


END_NAMESPACE_MW


#endif

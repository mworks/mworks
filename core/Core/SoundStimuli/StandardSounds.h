/*
 *  StandardSounds.h
 *  MWorksCore
 *
 *  Created by David Cox on 10/13/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef STANDARD_SOUNDS_H_
#define STANDARD_SOUNDS_H_

#include "ComponentRegistry.h"

#ifndef MWORKS_EXCLUDE_LEGACY_SOUNDS
#include "LegacySounds.hpp"
#endif


BEGIN_NAMESPACE_MW


void registerStandardSounds(ComponentRegistry &registry);


END_NAMESPACE_MW


#endif /* STANDARD_SOUNDS_H_ */

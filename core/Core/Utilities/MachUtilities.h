/*
 *  MachUtilities.h
 *  MWorksCore
 *
 *  Created by Christopher Stawarz on 5/13/11.
 *  Copyright 2011 MIT. All rights reserved.
 *
 */

#ifndef _MW_MACH_UTILITIES_H
#define _MW_MACH_UTILITIES_H


#include "MWorksMacros.h"


BEGIN_NAMESPACE_MW


int set_realtime(int period, int computation, int constraint);
int set_realtime(int priority);


END_NAMESPACE_MW


#endif

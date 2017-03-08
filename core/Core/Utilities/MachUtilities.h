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

#include <mach/mach_error.h>
#include <mach/mach_time.h>

#include "MWorksTypes.h"


BEGIN_NAMESPACE_MW


int set_realtime(int period, int computation, int constraint);
int set_realtime(int priority);
bool logMachError(const char *functionName, mach_error_t error);


class MachTimebase {
    
public:
    MachTimebase() : timebaseInfo(getTimebaseInfo()) { }
    
    MWTime absoluteToNanos(uint64_t absolute) const {
        return absolute * timebaseInfo.numer / timebaseInfo.denom;
    }
    
    uint64_t nanosToAbsolute(MWTime nanos) const {
        return nanos * timebaseInfo.denom / timebaseInfo.numer;
    }
    
private:
    static mach_timebase_info_data_t getTimebaseInfo();
    
    const mach_timebase_info_data_t timebaseInfo;
    
};


END_NAMESPACE_MW


#endif



























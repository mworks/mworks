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

#include <boost/noncopyable.hpp>

#include "Scheduler.h"


BEGIN_NAMESPACE_MW


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


class MachThreadSelf: boost::noncopyable {
    
public:
    explicit MachThreadSelf(const std::string &name);
    ~MachThreadSelf();
    
    bool setPriority(TaskPriority priority);
    
    bool setQOSClass(qos_class_t qosClass, int relativePriority = 0);
    bool setImportance(integer_t importance);
    bool setTimesharing(bool timesharing);
    bool setRealtime(uint32_t period, uint32_t computation, uint32_t constraint, bool preemptible = true);
    
private:
    const thread_port_t port;
    
};


void logMachThreadInfo();


inline void throwMachError(const char *functionName, mach_error_t error) {
    if (error != ERR_SUCCESS) {
        throw SimpleException(M_SYSTEM_MESSAGE_DOMAIN,
                              boost::format("%s failed: %s (%d)") % functionName % mach_error_string(error) % error);
    }
}


inline bool logMachError(const char *functionName, mach_error_t error) {
    const bool failed = (error != ERR_SUCCESS);
    if (failed) {
        merror(M_SYSTEM_MESSAGE_DOMAIN, "%s failed: %s (%d)", functionName, mach_error_string(error), error);
    }
    return failed;
}


END_NAMESPACE_MW


#endif

/*
 *  MachUtilities.cpp
 *  MWorksCore
 *
 *  Created by Christopher Stawarz on 5/13/11.
 *  Copyright 2011 MIT. All rights reserved.
 *
 */

#include "MachUtilities.h"

#include <mach/mach.h>

#include "Utilities.h"


BEGIN_NAMESPACE_MW


mach_timebase_info_data_t MachTimebase::getTimebaseInfo() {
    mach_timebase_info_data_t timebaseInfo;
    if (logMachError("mach_timebase_info", mach_timebase_info(&timebaseInfo))) {
        throw SimpleException(M_SYSTEM_MESSAGE_DOMAIN, "Unable to create MachTimebase");
    }
    return timebaseInfo;
}


MachThreadSelf::MachThreadSelf(const std::string &name) :
    port(mach_thread_self())
{
    pthread_setname_np(name.c_str());
}


MachThreadSelf::~MachThreadSelf() {
    //
    // NOTE: Although Apple's docs don't mention it, it's crucial to release the port returned by
    // mach_thread_self with mach_port_deallocate.  Otherwise, the port is leaked.  See
    // http://technosloth.blogspot.com/2010/08/unexpected-mach-port-refcounts.html
    //
    if (logMachError("mach_port_deallocate", mach_port_deallocate(mach_task_self(), port))) {
        mwarning(M_SYSTEM_MESSAGE_DOMAIN,
                 "Unable to release Mach thread self port; application is leaking system resources");
    }
}


//
// MachThreadSelf::setPriority attempts to map each MWorks TaskPriority level to an appropriate
// kernel scheduler priority.
//
// The scheduler priorities available to non-realtime, user-space threads range from 0 (MINPRI_USER)
// to 63 (MAXPRI_USER).  By default, user-space threads have priority 31 (BASEPRI_DEFAULT).
// Inspection (using logMachThreadInfo; see below) on macOS 10.14.3 and iOS 12.1.4 reveals that the
// application main thread runs at priority 46 (BASEPRI_BACKGROUND) or 47 (BASEPRI_FOREGROUND),
// depending on whether the application is currently frontmost.  Inspection also shows that
// the thread created by a CVDisplayLink has priority 54 and runs in non-timesharing mode.
//
// Based on these observations, we have chosen MW_STIMULUS_PRIORITY to match that used by
// CVDisplayLink (54), and we have assigned MW_IODEVICE_PRIORITY and MW_DEFAULT_PRIORITY to the two
// levels immediately below MW_STIMULUS_PRIORITY (53 and 52, respectively).  Consequently, each of
// these three priorities
//   1. is always above that of the main thread and
//   2. lies in the most elevated band of user-space priorities (52-63).
//
// Finally, note that we do *not* adjust the priority of network tasks.  For those, we simply accept
// the system default.
//
// TODO: We should periodically check whether the priorities of system-created threads still match
// our previous observations.
//
// References:
//   * _Mac OS X Internals: A Systems Approach_ by Amit Singh (http://a.co/1igATUN )
//   * osfmk/kern/sched.h in the XNU kernel source (available from https://opensource.apple.com )
//


namespace {
    constexpr int BASEPRI_DEFAULT      = 31;  // From sched.h
    constexpr int MW_DEFAULT_PRIORITY  = 52;
    constexpr int MW_IODEVICE_PRIORITY = 53;
    constexpr int MW_STIMULUS_PRIORITY = 54;
}


bool MachThreadSelf::setPriority(TaskPriority priority) {
    //
    // NOTE: The kernel computes a thread's priority by adding BASEPRI_DEFAULT to its importance
    // value.  Therefore, to obtain our desired thread priorities, we must *subtract* BASEPRI_DEFAULT
    // from them to obtain the appropriate importance values.
    //
    switch (priority) {
        case TaskPriority::Network:
            // No adjustment
            return true;
            
        case TaskPriority::Default:
            return setImportance(MW_DEFAULT_PRIORITY - BASEPRI_DEFAULT);
            
        case TaskPriority::IODevice:
            // Since I/O device threads typically perform a fixed amount of work at regular intervals,
            // they are good candidates for non-timesharing scheduling
            return (setImportance(MW_IODEVICE_PRIORITY - BASEPRI_DEFAULT) &&
                    setTimesharing(false));
            
        case TaskPriority::Stimulus:
            // Copy the configuration of CVDisplayLink threads.  This is primarily for the benefit of
            // the CADisplayLink thread spawned by IOSStimulusDisplay.
            return (setImportance(MW_STIMULUS_PRIORITY - BASEPRI_DEFAULT) &&
                    setTimesharing(false));
    }
}


bool MachThreadSelf::setQOSClass(qos_class_t qosClass, int relativePriority) {
    const auto errorNum = pthread_set_qos_class_self_np(qosClass, relativePriority);
    if (0 != errorNum) {
        merror(M_SYSTEM_MESSAGE_DOMAIN,
               "pthread_set_qos_class_self_np failed: %s (%d)",
               strerror(errorNum),
               errorNum);
        return false;
    }
    return true;
}


bool MachThreadSelf::setImportance(integer_t importance) {
    thread_precedence_policy_data_t precedencePolicyData;
    precedencePolicyData.importance = importance;
    
    return !logMachError("thread_policy_set",
                         thread_policy_set(port,
                                           THREAD_PRECEDENCE_POLICY,
                                           reinterpret_cast<thread_policy_t>(&precedencePolicyData),
                                           THREAD_PRECEDENCE_POLICY_COUNT));
}


bool MachThreadSelf::setTimesharing(bool timesharing) {
    thread_extended_policy_data_t extendedPolicyData;
    extendedPolicyData.timeshare = timesharing;
    
    return !logMachError("thread_policy_set",
                         thread_policy_set(port,
                                           THREAD_EXTENDED_POLICY,
                                           reinterpret_cast<thread_policy_t>(&extendedPolicyData),
                                           THREAD_EXTENDED_POLICY_COUNT));
}


bool MachThreadSelf::setRealtime(uint32_t period, uint32_t computation, uint32_t constraint, bool preemptible) {
    thread_time_constraint_policy_data_t timeConstraintPolicyData;
    timeConstraintPolicyData.period = period;
    timeConstraintPolicyData.computation = computation;
    timeConstraintPolicyData.constraint = constraint;
    timeConstraintPolicyData.preemptible = preemptible;
    
    return !logMachError("thread_policy_set",
                         thread_policy_set(port,
                                           THREAD_TIME_CONSTRAINT_POLICY,
                                           reinterpret_cast<thread_policy_t>(&timeConstraintPolicyData),
                                           THREAD_TIME_CONSTRAINT_POLICY_COUNT));
}


void logMachThreadInfo() {
    //
    // Log (to stderr) the scheduling information for all threads in the current process
    //
    
    thread_array_t threadList;
    mach_msg_type_number_t threadCount;
    if (logMachError("task_threads", task_threads(mach_task_self(), &threadList, &threadCount))) {
        return;
    }
    
    for (int threadNumber = 0; threadNumber < threadCount; threadNumber++) {
        fprintf(stderr, "\nThread %d", threadNumber + 1);
        
        const auto thread = threadList[threadNumber];
        mach_msg_type_number_t policyInfoCount;
        boolean_t getDefault;
        
        if (auto pthread = pthread_from_mach_thread_np(thread)) {
            qos_class_t qosClass;
            int relativePriority;
            if (0 == pthread_get_qos_class_np(pthread, &qosClass, &relativePriority) &&
                QOS_CLASS_UNSPECIFIED != qosClass)
            {
                fprintf(stderr, "\n  QoS class: ");
                switch (qosClass) {
                    case QOS_CLASS_USER_INTERACTIVE:
                        fprintf(stderr, "USER INTERACTIVE");
                        break;
                    case QOS_CLASS_USER_INITIATED:
                        fprintf(stderr, "USER INITIATED");
                        break;
                    case QOS_CLASS_DEFAULT:
                        fprintf(stderr, "DEFAULT");
                        break;
                    case QOS_CLASS_UTILITY:
                        fprintf(stderr, "UTILITY");
                        break;
                    case QOS_CLASS_BACKGROUND:
                        fprintf(stderr, "BACKGROUND");
                        break;
                    default:
                        fprintf(stderr, "Unknown (%u)", qosClass);
                        break;
                }
                if (relativePriority != 0) {
                    fprintf(stderr, "\n  QoS relative priority: %d", relativePriority);
                }
            }
        }
        
        {
            thread_precedence_policy_data_t precedencePolicyData;
            policyInfoCount = THREAD_PRECEDENCE_POLICY_COUNT;
            getDefault = FALSE;
            if (KERN_SUCCESS == thread_policy_get(thread,
                                                  THREAD_PRECEDENCE_POLICY,
                                                  reinterpret_cast<thread_policy_t>(&precedencePolicyData),
                                                  &policyInfoCount,
                                                  &getDefault))
            {
                if (!getDefault) {
                    fprintf(stderr, "\n  importance = %d", precedencePolicyData.importance);
                }
            }
        }
        
        {
            thread_extended_policy_data_t extendedPolicyData;
            policyInfoCount = THREAD_EXTENDED_POLICY_COUNT;
            getDefault = FALSE;
            if (KERN_SUCCESS == thread_policy_get(thread,
                                                  THREAD_EXTENDED_POLICY,
                                                  reinterpret_cast<thread_policy_t>(&extendedPolicyData),
                                                  &policyInfoCount,
                                                  &getDefault))
            {
                if (!getDefault) {
                    fprintf(stderr, "\n  timeshare: %s", (extendedPolicyData.timeshare ? "YES" : "NO"));
                }
            }
        }
        
        {
            thread_time_constraint_policy_data_t timeConstraintPolicyData;
            policyInfoCount = THREAD_TIME_CONSTRAINT_POLICY_COUNT;
            getDefault = FALSE;
            if (KERN_SUCCESS == thread_policy_get(thread,
                                                  THREAD_TIME_CONSTRAINT_POLICY,
                                                  reinterpret_cast<thread_policy_t>(&timeConstraintPolicyData),
                                                  &policyInfoCount,
                                                  &getDefault))
            {
                if (!getDefault) {
                    fprintf(stderr,
                            "\n  realtime:"
                            "\n    period = %u"
                            "\n    computation = %u"
                            "\n    constraint = %u"
                            "\n    preemptible: %s",
                            timeConstraintPolicyData.period,
                            timeConstraintPolicyData.computation,
                            timeConstraintPolicyData.constraint,
                            (timeConstraintPolicyData.preemptible ? "YES" : "NO"));
                }
            }
        }
        
        {
            thread_extended_info_data_t extendedInfo;
            mach_msg_type_number_t extendedInfoCount = THREAD_EXTENDED_INFO_COUNT;
            if (KERN_SUCCESS == thread_info(thread,
                                            THREAD_EXTENDED_INFO,
                                            reinterpret_cast<thread_info_t>(&extendedInfo),
                                            &extendedInfoCount))
            {
                fprintf(stderr,
                        "\n  priority         = %d"
                        "\n  current priority = %d"
                        "\n  maximum priority = %d"
                        "\n  name: \"%s\"",
                        extendedInfo.pth_priority,
                        extendedInfo.pth_curpri,
                        extendedInfo.pth_maxpriority,
                        extendedInfo.pth_name);
            }
        }
        
        fprintf(stderr, "\n");
    }
    
    fflush(stderr);
}


bool logMachError(const char *functionName, mach_error_t error) {
    const bool failed = (error != ERR_SUCCESS);
    if (failed) {
        merror(M_SYSTEM_MESSAGE_DOMAIN, "%s failed: %s (%d)", functionName, mach_error_string(error), error);
    }
    return failed;
}


END_NAMESPACE_MW




























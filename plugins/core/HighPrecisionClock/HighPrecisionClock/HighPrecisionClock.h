//
//  HighPrecisionClock.h
//  HighPrecisionClock
//
//  Created by Christopher Stawarz on 4/12/13.
//  Copyright (c) 2013 The MWorks Project. All rights reserved.
//

#ifndef __HighPrecisionClock__HighPrecisionClock__
#define __HighPrecisionClock__HighPrecisionClock__


BEGIN_NAMESPACE_MW


class HighPrecisionClock : public Clock, boost::noncopyable {
    
public:
    HighPrecisionClock();
    ~HighPrecisionClock();
    
    MWTime getSystemBaseTimeNS() MW_OVERRIDE;
    MWTime getSystemTimeNS() MW_OVERRIDE;
    MWTime getCurrentTimeNS() MW_OVERRIDE;
	
    void startClock() MW_OVERRIDE;
    void stopClock() MW_OVERRIDE;
    
    void sleepNS(MWTime time) MW_OVERRIDE;
    
private:
    static const MWTime periodUS = 200LL;
    static const MWTime computationUS = 50LL;
    
    static bool logMachError(const char *functionName, mach_error_t error) {
        const bool failed = (error != ERR_SUCCESS);
        if (failed) {
            merror(M_SCHEDULER_MESSAGE_DOMAIN, "%s failed: %s (%d)", functionName, mach_error_string(error), error);
        }
        return failed;
    }
    
    static mach_timebase_info_data_t getTimebaseInfo();
    static void destroySemaphore(semaphore_t *sem);
    
    MWTime absoluteToNanos(uint64_t absolute) const {
        return absolute * timebaseInfo.numer / timebaseInfo.denom;
    }
    
    uint64_t nanosToAbsolute(MWTime nanos) const {
        return nanos * timebaseInfo.denom / timebaseInfo.numer;
    }
    
    bool isRunning() const { return (runLoopThread.get_id() != boost::thread::id()); }
    void runLoop();
    
    const mach_timebase_info_data_t timebaseInfo;
    const uint64_t systemBaseTimeAbsolute;
    
    boost::thread runLoopThread;
    
    class WaitInfo {
    public:
        WaitInfo(uint64_t expirationTime, semaphore_t semaphore) :
            expirationTime(expirationTime),
            semaphore(semaphore)
        { }
        
        uint64_t getExpirationTime() const { return expirationTime; }
        semaphore_t getSemaphore() const { return semaphore; }
        
        bool operator<(const WaitInfo &rhs) const {
            // Use greater-than so that the instance with the smaller expiration time is closer to
            // the top of the priority queue
            return (expirationTime > rhs.expirationTime);
        }
        
    private:
        uint64_t expirationTime;
        semaphore_t semaphore;
    };
    
    boost::thread_specific_ptr<semaphore_t> threadSpecificSemaphore;
    std::priority_queue<WaitInfo> waits;
    boost::mutex waitsMutex;
    typedef boost::lock_guard<boost::mutex> lock_guard;
    
};


END_NAMESPACE_MW


#endif // !defined(__HighPrecisionClock__HighPrecisionClock__)



























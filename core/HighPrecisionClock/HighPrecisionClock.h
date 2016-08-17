//
//  HighPrecisionClock.h
//  HighPrecisionClock
//
//  Created by Christopher Stawarz on 4/12/13.
//  Copyright (c) 2013 The MWorks Project. All rights reserved.
//

#ifndef __HighPrecisionClock__HighPrecisionClock__
#define __HighPrecisionClock__HighPrecisionClock__

#include "SimpleClock.hpp"


BEGIN_NAMESPACE_MW


class HighPrecisionClock : public SimpleClock {
    
public:
    HighPrecisionClock();
    ~HighPrecisionClock();
	
    void startClock() MW_OVERRIDE;
    void stopClock() MW_OVERRIDE;
    
    void sleepNS(MWTime time) MW_OVERRIDE;
    void yield() MW_OVERRIDE;
    
private:
    static const MWTime periodUS = 200LL;
    static const MWTime computationUS = 50LL;
    
    static void destroySemaphore(semaphore_t *sem);
    
    bool isRunning() const { return (runLoopThread.get_id() != boost::thread::id()); }
    void wait(uint64_t expirationTime = 0);
    void runLoop();
    
    const uint64_t period;
    const uint64_t computation;
    
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



























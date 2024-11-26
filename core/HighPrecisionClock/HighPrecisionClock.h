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
	
    void startClock() override;
    void stopClock() override;
    
    void sleepNS(MWTime time) override;
    void yield() override;
    
private:
    static constexpr MWTime periodUS = 200LL;
    static constexpr MWTime computationUS = 50LL;
    
    bool isRunning() const { return runLoopThread.joinable(); }
    void wait(uint64_t expirationTime = 0);
    void runLoop();
    
    const uint64_t period;
    const uint64_t computation;
    
    std::atomic_bool running;
    static_assert(decltype(running)::is_always_lock_free);
    std::thread runLoopThread;
    
    struct WaitInfo;
    std::priority_queue<WaitInfo> waits;
    using lock_guard = std::lock_guard<UnfairMutex>;
    lock_guard::mutex_type waitsMutex;
    
    struct Semaphore : boost::noncopyable {
        Semaphore();
        ~Semaphore();
        void wait();
        void signal();
    private:
        semaphore_t semaphore;
    };
    
    struct WaitInfo {
        WaitInfo(uint64_t expirationTime, Semaphore &semaphore) :
            expirationTime(expirationTime),
            semaphore(&semaphore)
        { }
        
        uint64_t getExpirationTime() const { return expirationTime; }
        Semaphore & getSemaphore() const { return *semaphore; }
        
        bool operator<(const WaitInfo &rhs) const {
            // Use greater-than so that the instance with the smaller expiration time is closer to
            // the top of the priority queue
            return (expirationTime > rhs.expirationTime);
        }
        
    private:
        uint64_t expirationTime;
        Semaphore *semaphore;
    };
    
};


END_NAMESPACE_MW


#endif // !defined(__HighPrecisionClock__HighPrecisionClock__)

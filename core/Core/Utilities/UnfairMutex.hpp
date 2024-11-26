//
//  UnfairMutex.hpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 11/26/24.
//

#ifndef UnfairMutex_h
#define UnfairMutex_h

#include <os/lock.h>

#include <boost/noncopyable.hpp>

#include "MWorksMacros.h"


BEGIN_NAMESPACE_MW


class UnfairMutex : boost::noncopyable {
    
public:
    void lock() { os_unfair_lock_lock(&mutex); }
    bool try_lock() { return os_unfair_lock_trylock(&mutex); }
    void unlock() { os_unfair_lock_unlock(&mutex); }
    
private:
    os_unfair_lock mutex = OS_UNFAIR_LOCK_INIT;
    
};


END_NAMESPACE_MW


#endif /* UnfairMutex_h */

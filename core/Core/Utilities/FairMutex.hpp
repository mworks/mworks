//
//  FairMutex.hpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 4/10/19.
//

#ifndef FairMutex_hpp
#define FairMutex_hpp

#include <pthread.h>

#include <boost/noncopyable.hpp>

#include "MWorksMacros.h"


BEGIN_NAMESPACE_MW


class FairMutex : boost::noncopyable {
    
public:
    FairMutex();
    ~FairMutex();
    
    void lock();
    bool try_lock();
    void unlock();
    
private:
    pthread_mutex_t mutex;
    
};


END_NAMESPACE_MW


#endif /* FairMutex_hpp */

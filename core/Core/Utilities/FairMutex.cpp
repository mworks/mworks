//
//  FairMutex.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 4/10/19.
//

#include "FairMutex.hpp"

#include "Utilities.h"


BEGIN_NAMESPACE_MW


FairMutex::FairMutex() {
    int status;
    pthread_mutexattr_t attr;
    if (0 == (status = pthread_mutexattr_init(&attr))) {
        if (0 == (status = pthread_mutexattr_setpolicy_np(&attr, PTHREAD_MUTEX_POLICY_FAIRSHARE_NP))) {
            status = pthread_mutex_init(&mutex, &attr);
        }
        (void)pthread_mutexattr_destroy(&attr);
    }
    if (0 != status) {
        throw SimpleException(M_SYSTEM_MESSAGE_DOMAIN, "Cannot create pthread mutex", strerror(status));
    }
}


FairMutex::~FairMutex() {
    auto status = pthread_mutex_destroy(&mutex);
    if (0 != status) {
        merror(M_SYSTEM_MESSAGE_DOMAIN, "Cannot destroy pthread mutex: %s", strerror(status));
    }
}


void FairMutex::lock() {
    auto status = pthread_mutex_lock(&mutex);
    if (0 != status) {
        throw SimpleException(M_SYSTEM_MESSAGE_DOMAIN, "Cannot lock pthread mutex", strerror(status));
    }
}


bool FairMutex::try_lock() {
    auto status = pthread_mutex_trylock(&mutex);
    switch (status) {
        case 0:
            return true;
        case EBUSY:
            return false;
        default:
            throw SimpleException(M_SYSTEM_MESSAGE_DOMAIN, "trylock failed on pthread mutex", strerror(status));
    }
}


void FairMutex::unlock() {
    auto status = pthread_mutex_unlock(&mutex);
    if (0 != status) {
        merror(M_SYSTEM_MESSAGE_DOMAIN, "Cannot unlock pthread mutex: %s", strerror(status));
    }
}


END_NAMESPACE_MW

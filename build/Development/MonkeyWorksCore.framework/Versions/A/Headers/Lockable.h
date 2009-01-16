/**
 * Lockable.h
 *
 * Description: A thread mutex.
 *
 * History:
 *
 * Copyright 2004 MIT. All rights reserved.
 */
 
#ifndef _LOCKABLE_H
#define _LOCKABLE_H

#include <pthread.h>
namespace mw {
// macros for locking gets and sets
#define SAFE_GET(type, A)   type result; lock(); result = A; unlock(); return result;
#define SAFE_SET(A, B)	   lock(); A = B; unlock();

//#define VERBOSE_LOCKING 0
#ifdef VERBOSE_LOCKING
    #define M_ISLOCK mdebug("Locking in file %s at line %d", __FILE__, __LINE__); lock();
    #define M_HASLOCK(type) mdebug("Locking in file %s at line %d", __FILE__, __LINE__); type->lock();
    
    #define M_ISUNLOCK mdebug("Unlocking in file %s at line %d", __FILE__, __LINE__); unlock();
    #define M_HASUNLOCK(type) mdebug("Locking in file %s at line %d", __FILE__, __LINE__); type->unlock();
#else
    #define M_ISLOCK lock();
    #define M_HASLOCK(type) type->lock();
    
    #define M_ISUNLOCK unlock();
    #define M_HASUNLOCK(type) type->unlock();
#endif

class Lockable {
    private:
        // the mutex to lock on
        pthread_mutex_t mutex;

    public:
        /**
         * Initializees the mutex.
         */
        Lockable();
        /**
         * Destroys the mutex.  The destructor is virtual so that this class 
         * can be safely used as a parent class.
         */ 
        ~Lockable();
                
        /**
         * Locks the mutex.
         */
		inline void lock(){
			pthread_mutex_lock(&mutex);
		}
        
        /**
         * Unlocks the mutex.
         */
		inline void unlock(){
			pthread_mutex_unlock(&mutex);
		}

};
}
#endif
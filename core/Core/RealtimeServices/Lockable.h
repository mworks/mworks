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

#include <boost/thread/mutex.hpp>

#include "MWorksMacros.h"


BEGIN_NAMESPACE_MW


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
        boost::mutex theMutex;

    public:
        Lockable() { }
        virtual ~Lockable() { }
    
        Lockable(const Lockable &other) {
            // Don't attempt to copy the mutex
        }
    
        Lockable& operator=(const Lockable &other) {
            // Don't attempt to copy the mutex
            return (*this);
        }
                
        /**
         * Locks the mutex.
         */
		inline void lock(){
			theMutex.lock();
		}
        
        /**
         * Unlocks the mutex.
         */
		inline void unlock(){
			theMutex.unlock();
		}
    
        friend class Locker;

};


class Locker {
    
public:
    explicit Locker(Lockable &lockable) : theLock(lockable.theMutex) { }
    
private:
    boost::mutex::scoped_lock theLock;
    
};


END_NAMESPACE_MW


#endif
























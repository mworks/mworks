//
//  CFObjectPtr.h
//  MWorksCore
//
//  Created by Christopher Stawarz on 5/6/13.
//  Copyright (c) 2013 The MWorks Project. All rights reserved.
//

#ifndef MWorksCore_CFObjectPtr_h
#define MWorksCore_CFObjectPtr_h

#include <new>
#include <utility>

#include <CoreFoundation/CoreFoundation.h>

#include <MWorksCore/MWorksMacros.h>


BEGIN_NAMESPACE_MW
BEGIN_NAMESPACE(cf)


template <typename T>
inline void retainObject(T ref) { if (ref) CFRetain(ref); }


template <typename T>
inline void releaseObject(T ref) { if (ref) CFRelease(ref); }


template <typename T>
class ObjectPtr {
    
public:
    static ObjectPtr created(T ref) {
        if (!ref) throw std::bad_alloc();
        return owned(ref);
    }
    
    static ObjectPtr owned(T ref) {
        return ObjectPtr(ref, false);  // Already owned, so don't retain
    }
    
    static ObjectPtr borrowed(T ref) {
        return ObjectPtr(ref, true);   // Borrowed, so retain to establish ownership
    }
    
    ~ObjectPtr() {
        releaseObject<T>(ref);
    }
    
    constexpr ObjectPtr() noexcept :
        ref(nullptr)
    { }
    
    // Copy constructor
    ObjectPtr(const ObjectPtr &other) :
        ref(other.ref)
    {
        retainObject<T>(ref);
    }
    
    // Move constructor
    ObjectPtr(ObjectPtr &&other) noexcept :
        ref(other.ref)
    {
        other.ref = nullptr;
    }
    
    // Copy assignment
    ObjectPtr& operator=(const ObjectPtr &other) {
        if (this != &other) {
            releaseObject<T>(ref);
            ref = other.ref;
            retainObject<T>(ref);
        }
        return (*this);
    }
    
    // Move assignment
    ObjectPtr& operator=(ObjectPtr &&other) noexcept {
        std::swap(ref, other.ref);
        return (*this);
    }
    
    T get() const noexcept CF_RETURNS_NOT_RETAINED {
        return ref;
    }
    
    T release() noexcept CF_RETURNS_RETAINED {
        T _ref = ref;
        ref = nullptr;
        return _ref;
    }
    
    void reset() {
        (*this) = ObjectPtr();
    }
    
    explicit operator bool() const noexcept {
        return ref;
    }
    
private:
    ObjectPtr(T ref, bool retain) :
        ref(ref)
    {
        if (retain) retainObject<T>(ref);
    }
    
    T ref;
    
};


using ArrayPtr = ObjectPtr<CFArrayRef>;
using DataPtr = ObjectPtr<CFDataRef>;
using DictionaryPtr = ObjectPtr<CFDictionaryRef>;
using NumberPtr = ObjectPtr<CFNumberRef>;
using SetPtr = ObjectPtr<CFSetRef>;
using StringPtr = ObjectPtr<CFStringRef>;


END_NAMESPACE(cf)
END_NAMESPACE_MW


#endif /* !defined(MWorksCore_CFObjectPtr_h) */



























//
//  Utilities.h
//  MATLABTools
//
//  Created by Christopher Stawarz on 12/10/12.
//  Copyright (c) 2012 MWorks Project. All rights reserved.
//

#ifndef MATLABTools_Utilities_h
#define MATLABTools_Utilities_h

#include <matrix.h>

#include <boost/format.hpp>
#include <boost/move/move.hpp>

#include <MWorksCore/MWorksMacros.h>

#ifdef MATLAB_MEX_FILE
#  include <mex.h>
#else
#  include <new>
#  include <MWorksCore/Exceptions.h>
#endif


#define BEGIN_NAMESPACE_MW_MATLAB  BEGIN_NAMESPACE_MW    BEGIN_NAMESPACE(matlab)
#define END_NAMESPACE_MW_MATLAB    END_NAMESPACE(matlab) END_NAMESPACE_MW


BEGIN_NAMESPACE_MW_MATLAB


#ifndef MATLAB_MEX_FILE
class MATLABError : public SimpleException {
    
public:
    MATLABError(const std::string &message) :
        SimpleException(message)
    { }
    
};
#endif


inline void throwMATLABError(const std::string &errorID, const std::string &msg) {
#ifdef MATLAB_MEX_FILE
    mexErrMsgIdAndTxt(errorID.c_str(), "%s", msg.c_str());
#else
    throw MATLABError(msg);
#endif
}


inline void throwMATLABError(const std::string &errorID, const boost::format &fmt) {
    throwMATLABError(errorID, fmt.str());
}


struct null_ok_t {};
static const null_ok_t null_ok = null_ok_t();

struct throw_if_null_t {};
static const throw_if_null_t throw_if_null = throw_if_null_t();


class ArrayPtr {
    
    BOOST_MOVABLE_BUT_NOT_COPYABLE(ArrayPtr)
    
public:
    ~ArrayPtr() {
#ifdef MATLAB_MEX_FILE
        // If we're in a MEX-function, we don't deallocate the array in the destructor, as per the MATLAB docs:
        // http://www.mathworks.com/help/matlab/matlab_external/troubleshooting-mex-files.html#brhj1k9-1
#else
        if (ptr) mxDestroyArray(ptr);
#endif
    }
    
    ArrayPtr() : ptr(NULL) { }
    
    ArrayPtr(null_ok_t null_ok, mxArray *ptr) : ptr(ptr) { }
    
    ArrayPtr(throw_if_null_t throw_if_null, mxArray *ptr) :
        ptr(ptr)
    {
#ifdef MATLAB_MEX_FILE
        // MATLAB automatically terminates a MEX-function if memory allocation fails.
        // This is just a sanity check.
        mxAssert(ptr, "ptr is NULL");
#else
        if (!ptr) {
            throw std::bad_alloc();
        }
#endif
    }
    
    // Move constructor
    ArrayPtr(BOOST_RV_REF(ArrayPtr) other) :
        ptr(other.ptr)
    {
        other.ptr = NULL;
    }
    
    // Move assignment
    ArrayPtr& operator=(BOOST_RV_REF(ArrayPtr) other) {
        if (ptr) mxDestroyArray(ptr);
        ptr = other.ptr;
        other.ptr = NULL;
        return (*this);
    }
    
    bool isNull() const MW_NOEXCEPT {
        return (ptr == NULL);
    }
    
    mxArray* get() const MW_NOEXCEPT {
        return ptr;
    }
    
    mxArray* release() MW_NOEXCEPT {
        mxArray *orig_ptr = ptr;
        ptr = NULL;
        return orig_ptr;
    }
    
    void destroy() {
        mxAssert(ptr, "ptr is NULL");
        mxDestroyArray(ptr);
        ptr = NULL;
    }
    
private:
    mxArray *ptr;
    
};


END_NAMESPACE_MW_MATLAB


#endif  // !defined(MATLABTools_Utilities_h)



























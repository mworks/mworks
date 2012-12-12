//
//  Utilities.h
//  MATLABTools
//
//  Created by Christopher Stawarz on 12/10/12.
//  Copyright (c) 2012 MWorks Project. All rights reserved.
//

#ifndef MATLABTools_Utilities_h
#define MATLABTools_Utilities_h

#ifndef MATLAB_MEX_FILE
#  include <new>
#endif

#include <matrix.h>

#include <boost/noncopyable.hpp>

#include <MWorksCore/MWorksMacros.h>


#define BEGIN_NAMESPACE_MW_MATLAB  BEGIN_NAMESPACE_MW    BEGIN_NAMESPACE(matlab)
#define END_NAMESPACE_MW_MATLAB    END_NAMESPACE(matlab) END_NAMESPACE_MW


BEGIN_NAMESPACE_MW_MATLAB


class ArrayPtr : boost::noncopyable {
    
public:
    explicit ArrayPtr(mxArray *ptr) : ptr(ptr) {
#ifndef MATLAB_MEX_FILE
        if (!ptr) throw std::bad_alloc();
#endif
    }
    
    ~ArrayPtr() {
#ifndef MATLAB_MEX_FILE
        if (ptr) mxDestroyArray(ptr);
#endif
    }
    
    mxArray* get() const MW_NOEXCEPT {
        return ptr;
    }
    
    mxArray* release() MW_NOEXCEPT {
        mxArray *orig_ptr = ptr;
        ptr = NULL;
        return orig_ptr;
    }
    
private:
    mxArray *ptr;
    
};


END_NAMESPACE_MW_MATLAB


#endif  // !defined(MATLABTools_Utilities_h)



























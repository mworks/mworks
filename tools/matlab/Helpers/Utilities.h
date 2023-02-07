//
//  Utilities.h
//  MATLABTools
//
//  Created by Christopher Stawarz on 12/10/12.
//  Copyright (c) 2012 MWorks Project. All rights reserved.
//

#ifndef MATLABTools_Utilities_h
#define MATLABTools_Utilities_h


BEGIN_NAMESPACE_MW_MATLAB


[[noreturn]] inline void throwMATLABError(const std::string &errorID, const std::string &msg) {
    mexErrMsgIdAndTxt(errorID.c_str(), "%s", msg.c_str());
    // mexErrMsgIdAndTxt should not return.  If we get here, something went very wrong.
    std::abort();
}


[[noreturn]] inline void throwMATLABError(const std::string &errorID, const boost::format &fmt) {
    throwMATLABError(errorID, fmt.str());
}


struct null_ok_t {};
static constexpr null_ok_t null_ok = null_ok_t();

struct throw_if_null_t {};
static constexpr throw_if_null_t throw_if_null = throw_if_null_t();


class ArrayPtr {
    
public:
    ~ArrayPtr() {
        // We're in a MEX-function, so don't deallocate the array in the destructor, as per the MATLAB docs:
        // https://www.mathworks.com/help/matlab/matlab_external/creating-c-mex-files.html#brg2lvt-1
    }
    
    constexpr ArrayPtr() noexcept : ptr(nullptr) { }
    
    ArrayPtr(null_ok_t null_ok, mxArray *ptr) noexcept : ptr(ptr) { }
    
    ArrayPtr(throw_if_null_t throw_if_null, mxArray *ptr) :
        ptr(ptr)
    {
        // MATLAB automatically terminates a MEX-function if memory allocation fails.
        // This is just a sanity check.
        mxAssert(ptr, "ptr is NULL");
    }
    
    // No copying
    ArrayPtr(const ArrayPtr &other) = delete;
    ArrayPtr& operator=(const ArrayPtr &other) = delete;
    
    // Move constructor
    ArrayPtr(ArrayPtr &&other) noexcept :
        ptr(other.ptr)
    {
        other.ptr = nullptr;
    }
    
    // Move assignment
    ArrayPtr& operator=(ArrayPtr &&other) {
        if (ptr) mxDestroyArray(ptr);
        ptr = other.ptr;
        other.ptr = nullptr;
        return (*this);
    }
    
    explicit operator bool() const noexcept {
        return ptr;
    }
    
    mxArray* get() const noexcept {
        return ptr;
    }
    
    mxArray* release() noexcept {
        auto _ptr = ptr;
        ptr = nullptr;
        return _ptr;
    }
    
    void destroy() {
        mxAssert(ptr, "ptr is NULL");
        mxDestroyArray(ptr);
        ptr = nullptr;
    }
    
private:
    mxArray *ptr;
    
};


END_NAMESPACE_MW_MATLAB


#endif  // !defined(MATLABTools_Utilities_h)

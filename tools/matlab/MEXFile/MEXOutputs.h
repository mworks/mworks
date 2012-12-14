//
//  MEXOutputs.h
//  MATLABTools
//
//  Created by Christopher Stawarz on 12/17/12.
//  Copyright (c) 2012 MWorks Project. All rights reserved.
//

#ifndef __MATLABTools__MEXOutputs__
#define __MATLABTools__MEXOutputs__

#include "Array.h"


BEGIN_NAMESPACE_MW_MATLAB


class MEXOutputs {
    
public:
    MEXOutputs(int nlhs, mxArray **plhs) :
        numOutputs(nlhs),
        outputs(plhs),
        currentOutput(-1)
    { }
    
    int count() const {
        return (numOutputs - (currentOutput + 1));
    }
    
    MEXOutputs& operator<<(BOOST_RV_REF(ArrayPtr) ptr) {
        next() = ptr.release();
        return (*this);
    }
    
    template<typename T>
    MEXOutputs& operator<<(const std::vector<T> &values) {
        return (*this << Array::createVector(values));
    }
    
    MEXOutputs& operator<<(boost::container::vector<ArrayPtr> &values) {
        return (*this << Array::createVector(values));
    }
    
private:
    static void needMoreOutputs();
    
    mxArray*& next() {
        if (count() < 1) needMoreOutputs();
        currentOutput++;
        return outputs[currentOutput];
    }
    
    const int numOutputs;
    mxArray ** const outputs;
    int currentOutput;
    
};


END_NAMESPACE_MW_MATLAB


#endif  // !defined(__MATLABTools__MEXOutputs__)




























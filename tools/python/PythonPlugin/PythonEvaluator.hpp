//
//  PythonEvaluator.hpp
//  PythonPlugin
//
//  Created by Christopher Stawarz on 10/18/17.
//  Copyright © 2017 MWorks Project. All rights reserved.
//

#ifndef PythonEvaluator_hpp
#define PythonEvaluator_hpp

#include "Utilities.h"


BEGIN_NAMESPACE_MW_PYTHON


class PythonEvaluator : boost::noncopyable {
    
public:
    struct EvalState : boost::noncopyable {
        EvalState();
        ~EvalState();
    private:
        ScopedGILAcquire sga;
        const int cwdfd;
    };
    
    explicit PythonEvaluator(const boost::filesystem::path &filePath);
    explicit PythonEvaluator(const std::string &code, bool isExpr = false);
    ~PythonEvaluator();
    
    bool exec();
    bool eval(Datum &result);
    
    using ArgIter = const std::vector<Datum>::const_iterator;
    bool call(Datum &result, ArgIter first, ArgIter last);
    
    // Must be called within the scope of an EvalState
    PyObject * eval() {
        return PyEval_EvalCode(reinterpret_cast<PyObject *>(codeObject), globalsDict, globalsDict);
    }
    
private:
    PyObject * const globalsDict;
    PyCodeObject * const codeObject;
    
};


END_NAMESPACE_MW_PYTHON


#endif /* PythonEvaluator_hpp */

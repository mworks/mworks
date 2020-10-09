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
    
    static PyObject * getGlobalsDict();
    
public:
    struct EvalState : boost::noncopyable {
        EvalState();
        ~EvalState();
    private:
        ScopedGILAcquire sga;
        const int cwdfd;
    };
    
    virtual ~PythonEvaluator() { }
    
    bool exec();
    bool eval(Datum &result);
    
    using ArgIter = const std::vector<Datum>::const_iterator;
    bool call(Datum &result, ArgIter first, ArgIter last);
    
    // Must be called within the scope of an EvalState
    PyObject * eval() { return run(); }
    
protected:
    PythonEvaluator() :
        globalsDict(getGlobalsDict())
    { }
    
    virtual PyObject * run() = 0;
    
    PyObject * const globalsDict;
    
};


class PythonFileEvaluator : public PythonEvaluator {
    
public:
    explicit PythonFileEvaluator(const boost::filesystem::path &filePath) :
        filename(filePath.string())
    { }
    
private:
    PyObject * run() override;
    
    const std::string filename;
    
};


class PythonStringEvaluator : public PythonEvaluator {
    
public:
    PythonStringEvaluator(const std::string &code, bool isExpr = false) :
        code(code),
        start(isExpr ? Py_eval_input : Py_file_input)
    { }
    
private:
    PyObject * run() override;
    
    const std::string code;
    const int start;
    
};


END_NAMESPACE_MW_PYTHON


#endif /* PythonEvaluator_hpp */

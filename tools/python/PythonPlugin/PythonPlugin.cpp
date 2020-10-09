//
//  PythonPlugin.cpp
//  PythonTools
//
//  Created by Christopher Stawarz on 6/1/15.
//  Copyright (c) 2015 MWorks Project. All rights reserved.
//

#include "PythonFileResource.hpp"
#include "PythonImage.hpp"
#include "RunPythonFileAction.h"
#include "RunPythonStringAction.h"


BEGIN_NAMESPACE_MW_PYTHON


BEGIN_NAMESPACE()


Datum py_eval(const stx::SymbolTable::paramlist_type &paramlist)
{
    if (!(paramlist[0].isString())) {
        throw stx::BadFunctionCallException("Function PY_EVAL() requires a string parameter");
    }
    
    PythonStringEvaluator evaluator(paramlist[0].getString(), true);
    Datum result;
    if (evaluator.eval(result)) {
        return result;
    }
    
    return Datum(0);
}


Datum py_call(const stx::SymbolTable::paramlist_type &paramlist)
{
    if (paramlist.size() < 1) {
        throw stx::BadFunctionCallException("Function PY_CALL() requires at least one parameter");
    }
    
    if (!(paramlist[0].isString())) {
        throw stx::BadFunctionCallException("First parameter to function PY_CALL() must be a string");
    }
    
    PythonStringEvaluator evaluator(paramlist[0].getString(), true);
    Datum result;
    if (evaluator.call(result, paramlist.begin() + 1, paramlist.end())) {
        return result;
    }
    
    return Datum(0);
}


END_NAMESPACE()


class PythonPlugin : public Plugin {
    void registerComponents(boost::shared_ptr<ComponentRegistry> registry) override {
        registry->registerFactory<StandardComponentFactory, PythonFileResource>();
        registry->registerFactory<StandardStimulusFactory, PythonImage>();
        registry->registerFactory<StandardComponentFactory, RunPythonFileAction>();
        registry->registerFactory<StandardComponentFactory, RunPythonStringAction>();
        
        if (auto variableRegistry = global_variable_registry) {
            variableRegistry->setFunction("py_eval", 1, py_eval);
            variableRegistry->setFunction("py_call", -1, py_call);
        }
    }
};


extern "C" Plugin* getPlugin() {
    return new PythonPlugin();
}


END_NAMESPACE_MW_PYTHON

//
//  PythonEvaluator.hpp
//  PythonPlugin
//
//  Created by Christopher Stawarz on 10/18/17.
//  Copyright © 2017 MWorks Project. All rights reserved.
//

#ifndef PythonEvaluator_hpp
#define PythonEvaluator_hpp


BEGIN_NAMESPACE_MW


class PythonEvaluator : boost::noncopyable {
    
public:
    explicit PythonEvaluator(const boost::filesystem::path &filePath);
    explicit PythonEvaluator(const std::string &code);
    ~PythonEvaluator();
    
    bool eval();
    
private:
    PyObject * const globalsDict;
    PyCodeObject * const codeObject;
    
};


END_NAMESPACE_MW


#endif /* PythonEvaluator_hpp */





















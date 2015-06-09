//
//  PythonException.h
//  PythonTools
//
//  Created by Christopher Stawarz on 6/2/15.
//  Copyright (c) 2015 MWorks Project. All rights reserved.
//

#ifndef __PythonTools__PythonException__
#define __PythonTools__PythonException__


BEGIN_NAMESPACE_MW


class PythonException : public SimpleException {
    
public:
    static void logError(const std::string &message);
    
    explicit PythonException(const std::string &message);
    
};


END_NAMESPACE_MW


#endif /* !defined(__PythonTools__PythonException__) */

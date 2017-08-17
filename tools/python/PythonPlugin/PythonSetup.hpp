//
//  PythonSetup.hpp
//  PythonTools
//
//  Created by Christopher Stawarz on 8/17/17.
//  Copyright © 2017 MWorks Project. All rights reserved.
//

#ifndef PythonSetup_hpp
#define PythonSetup_hpp


BEGIN_NAMESPACE_MW
BEGIN_NAMESPACE(python)


//
// Returns a borrowed reference to __main__ module's dict.  On first call, also initializes
// the Python environment.
//
PyObject * getGlobalsDict();


END_NAMESPACE(python)
END_NAMESPACE_MW


#endif /* PythonSetup_hpp */

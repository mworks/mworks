//
//  PythonDataHelpers.h
//  MWorksCore
//
//  Created by Christopher Stawarz on 10/15/12.
//
//

#ifndef MWorksCore_PythonDataHelpers_h
#define MWorksCore_PythonDataHelpers_h

#include <Python.h>

#include <Scarab/scarab.h>


// Convert a Python object into a ScarabDatum
ScarabDatum *convert_python_to_scarab(PyObject *pObj);

// Convert a ScarabDatum into a corresponding Python object
PyObject *convert_scarab_to_python(ScarabDatum *datum, int prev_type = -1);


#endif /* !defined(MWorksCore_PythonDataHelpers_h) */

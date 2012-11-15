//
//  PythonDataHelpers.h
//
//  Created by Christopher Stawarz on 10/15/12.
//
//

#ifndef PythonDataHelpers_h_
#define PythonDataHelpers_h_

#include <boost/python.hpp>

#include <MWorksCore/GenericData.h>


BEGIN_NAMESPACE_MW


// Convert a Python object into a MWorks Datum
Datum convert_python_to_datum(const boost::python::object &obj);


// Convert a MWorks Datum into a Python object
boost::python::object convert_datum_to_python(const Datum &datum);


END_NAMESPACE_MW


#endif /* !defined(PythonDataHelpers_h_) */

//
//  PythonDataHelpers.h
//
//  Created by Christopher Stawarz on 10/15/12.
//
//

#ifndef PythonDataHelpers_h_
#define PythonDataHelpers_h_


BEGIN_NAMESPACE_MW


static inline boost::python::object manageNewRef(PyObject *pObj) {
    if (!pObj) {
        boost::python::throw_error_already_set();
    }
    return boost::python::object(boost::python::handle<>(pObj));
}


static inline boost::python::object manageBorrowedRef(PyObject *pObj) {
    if (!pObj) {
        boost::python::throw_error_already_set();
    }
    return boost::python::object(boost::python::handle<>(boost::python::borrowed(pObj)));
}


// Convert a Python object into a MWorks Datum
Datum convert_python_to_datum(const boost::python::object &obj);


// Convert a MWorks Datum into a Python object
boost::python::object convert_datum_to_python(const Datum &datum);


// Convert a long long value into a Python int or long, as appropriate
boost::python::object convert_longlong_to_python(long long ll_val);


END_NAMESPACE_MW


#endif /* !defined(PythonDataHelpers_h_) */

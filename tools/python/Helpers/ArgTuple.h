//
//  ArgTuple.h
//  PythonTools
//
//  Created by Christopher Stawarz on 10/8/19.
//  Copyright © 2019 MWorks Project. All rights reserved.
//

#ifndef ArgTuple_h
#define ArgTuple_h

#include "DataHelpers.h"


BEGIN_NAMESPACE_MW_PYTHON


class ArgTuple : boost::noncopyable {
    
public:
    explicit ArgTuple(PyObject *args) :
        args(args)
    { }
    
    template<typename... Args>
    void parse(Args&&... args) const {
        _parse(std::string(), std::make_tuple(), std::forward<Args>(args)...);
    }
    
private:
    template<typename Tuple, typename... Args>
    void _parse(std::string &&fmt, Tuple &&targets, std::string &str, Args&&... args) const {
        const char *utf8 = nullptr;
        _parse(fmt + "s", std::tuple_cat(targets, std::make_tuple(&utf8)), std::forward<Args>(args)...);
        str.assign(utf8);
    }
    
    template<typename Tuple, typename... Args>
    void _parse(std::string &&fmt, Tuple &&targets, int &i, Args&&... args) const {
        _parse(fmt + "i", std::tuple_cat(targets, std::make_tuple(&i)), std::forward<Args>(args)...);
    }
    
    template<typename Tuple, typename... Args>
    void _parse(std::string &&fmt, Tuple &&targets, long long &ll, Args&&... args) const {
        _parse(fmt + "L", std::tuple_cat(targets, std::make_tuple(&ll)), std::forward<Args>(args)...);
    }
    
    template<typename Tuple, typename... Args>
    void _parse(std::string &&fmt, Tuple &&targets, Py_ssize_t &sst, Args&&... args) const {
        _parse(fmt + "n", std::tuple_cat(targets, std::make_tuple(&sst)), std::forward<Args>(args)...);
    }
    
    template<typename Tuple, typename... Args>
    void _parse(std::string &&fmt, Tuple &&targets, ObjectPtr &obj, Args&&... args) const {
        PyObject *o = nullptr;
        _parse(fmt + "O", std::tuple_cat(targets, std::make_tuple(&o)), std::forward<Args>(args)...);
        obj = ObjectPtr::borrowed(o);
    }
    
    static int convertToDatum(PyObject *o, void *_datum) {
        auto &datum = *static_cast<Datum *>(_datum);
        datum = convertObjectToDatum(ObjectPtr::borrowed(o));
        return 1;
    }
    
    template<typename Tuple, typename... Args>
    void _parse(std::string &&fmt, Tuple &&targets, Datum &datum, Args&&... args) const {
        _parse(fmt + "O&",
               std::tuple_cat(targets, std::make_tuple(safeCall<convertToDatum, PyObject *, void *>, &datum)),
               std::forward<Args>(args)...);
    }
    
    template<typename Tuple, typename... Args>
    void _parse(std::string &&fmt, Tuple &&targets, bool &b, Args&&... args) const {
        int val = 0;
        _parse(fmt + "p", std::tuple_cat(targets, std::make_tuple(&val)), std::forward<Args>(args)...);
        b = val;
    }
    
    template<typename Tuple>
    void _parse(std::string &&fmt, Tuple &&targets) const {
        if (!std::apply(PyArg_ParseTuple, std::tuple_cat(std::make_tuple(args, fmt.c_str()), std::forward<Tuple>(targets)))) {
            throw ErrorAlreadySet();
        }
    }
    
    PyObject * const args;
    
};


END_NAMESPACE_MW_PYTHON


#endif /* ArgTuple_h */

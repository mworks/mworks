//
//  PythonEvaluator.cpp
//  PythonPlugin
//
//  Created by Christopher Stawarz on 10/18/17.
//  Copyright © 2017 MWorks Project. All rights reserved.
//

#include "PythonEvaluator.hpp"

#include <numpy/arrayobject.h>

#include "GILHelpers.h"
#include "PythonDataHelpers.h"
#include "PythonException.h"

using namespace boost::python;


BEGIN_NAMESPACE_MW


BEGIN_NAMESPACE()


inline cf::ObjectPtr<CFBundleRef> getBundle() {
    return cf::ObjectPtr<CFBundleRef>::borrowed(CFBundleGetBundleWithIdentifier(CFSTR(PRODUCT_BUNDLE_IDENTIFIER)));
}


std::string getPathForURL(const cf::ObjectPtr<CFURLRef> &url) {
    std::vector<char> path(1024);
    if (!CFURLGetFileSystemRepresentation(url.get(),
                                          true,
                                          reinterpret_cast<UInt8 *>(path.data()),
                                          path.size()))
    {
        throw SimpleException(M_PLUGIN_MESSAGE_DOMAIN, "Cannot convert URL to filesystem path");
    }
    return std::string(path.data());
}


class StaticExtensionModuleFinder {
    
public:
    StaticExtensionModuleFinder() :
        bundle(getBundle())
    { }
    
    std::string getExecutablePath() const {
        auto executableURL = cf::ObjectPtr<CFURLRef>::owned(CFBundleCopyExecutableURL(bundle.get()));
        if (!executableURL) {
            throw SimpleException(M_PLUGIN_MESSAGE_DOMAIN, "Cannot obtain path to Python plugin executable");
        }
        return getPathForURL(executableURL);
    }
    
    bool haveInitFuncForName(const std::string &name) const {
        auto functionName = cf::StringPtr::created(CFStringCreateWithCString(kCFAllocatorDefault,
                                                                             ("PyInit_" + name).c_str(),
                                                                             kCFStringEncodingUTF8));
        return CFBundleGetFunctionPointerForName(bundle.get(), functionName.get());
    }
    
private:
    const cf::ObjectPtr<CFBundleRef> bundle;
    
};


inline boost::shared_ptr<RegistryAwareEventStreamInterface> getCore() {
    return Server::instance();
}


boost::python::object getcodec() {
    auto core = getCore();
    boost::python::dict codec;
    for (auto &name : core->getVariableTagNames()) {
        auto code = core->lookupCodeForTag(name);
        if (code >= 0) {
            codec[code] = name;
        }
    }
    return codec;
}


boost::python::object get_reverse_codec() {
    auto core = getCore();
    boost::python::dict reverse_codec;
    for (auto &name : core->getVariableTagNames()) {
        auto code = core->lookupCodeForTag(name);
        if (code >= 0) {
            reverse_codec[name] = code;
        }
    }
    return reverse_codec;
}


boost::python::object getvar(const std::string &name) {
    auto var = getCore()->getVariable(name);
    if (!var) {
        throw UnknownVariableException(name);
    }
    return convert_datum_to_python(var->getValue());
}


void setvar(const std::string &name, const boost::python::object &value) {
    auto var = getCore()->getVariable(name);
    if (!var) {
        throw UnknownVariableException(name);
    }
    Datum val = convert_python_to_datum(value);
    {
        // setValue can perform an arbitrary number of notifications, some of which
        // may trigger blocking I/O operations, so temporarily release the GIL
        ScopedGILRelease sgr;
        var->setValue(val);
    }
}


class EventCallback {
    
public:
    ~EventCallback() {
        ScopedGILAcquire sga;
        delete callback;
    }
    
    explicit EventCallback(const boost::python::object &cb) {
        // The caller already holds the GIL, so we don't need to acquire it
        callback = new boost::python::object(cb);
    }
    
    // Copy constructor
    EventCallback(const EventCallback &other) {
        ScopedGILAcquire sga;
        callback = new boost::python::object(*other.callback);
    }
    
    // No move construction, copy assignment, or move assignment
    EventCallback(EventCallback &&other) = delete;
    EventCallback& operator=(const EventCallback &other) = delete;
    EventCallback& operator=(EventCallback &&other) = delete;
    
    void operator()(boost::shared_ptr<Event> evt) {
        ScopedGILAcquire sga;
        try {
            (*callback)(evt);
        } catch (const boost::python::error_already_set &) {
            PythonException::logError("Python event callback failed");
        }
    }
    
private:
    // Use a pointer so that we can ensure that the GIL is held when the callback is destroyed
    boost::python::object *callback = nullptr;
    
};


const std::string callbacksKey("<PythonPlugin:mworkscore.register_event_callback>");


//
// Before registering or unregistering event callbacks, we always release the GIL.  This
// prevents potential deadlocks due to lock acquisition order.
//
// For example, when a Python event callback is invoked, the event handler first acquires
// EventCallbackHandler::callbacks_lock and then the GIL.  If a Python event-registration
// function were invoked simultaneously, it would be holding the GIL first and then acquire
// callbacks_lock, thereby potenially creating a situation where each thread was waiting
// for the lock held by the other (i.e. deadlock).
//
// Note, however, that we must create EventCallback instances *before* we release the GIL.
//


void register_event_callback(const boost::python::object &callback) {
    EventCallback cb(callback);
    ScopedGILRelease sgr;
    getCore()->registerCallback(cb, callbacksKey);
}


void register_event_callback_for_name(const std::string &name, const boost::python::object &callback) {
    EventCallback cb(callback);
    ScopedGILRelease sgr;
    getCore()->registerCallback(name, cb, callbacksKey);
}


void register_event_callback_for_code(int code, const boost::python::object &callback) {
    EventCallback cb(callback);
    ScopedGILRelease sgr;
    getCore()->registerCallback(code, cb, callbacksKey);
}


void unregister_event_callbacks() {
    ScopedGILRelease sgr;
    getCore()->unregisterCallbacks(callbacksKey);
}


PyModuleDef mworkscoreModule = {
    PyModuleDef_HEAD_INIT,
    "mworkscore",
    nullptr,
    -1,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr
};


PyObject * init_mworkscore() {
    try {
        auto module = manageBorrowedRef( PyModule_Create(&mworkscoreModule) );
        boost::python::scope moduleScope(module);
        
        class_<StaticExtensionModuleFinder>("_StaticExtensionModuleFinder")
        .def("get_executable_path", &StaticExtensionModuleFinder::getExecutablePath)
        .def("have_init_func_for_name", &StaticExtensionModuleFinder::haveInitFuncForName)
        ;
        
        class_<Event>("Event", no_init)
        .add_property("code", &Event::getEventCode)
        .add_property("time", extractEventTime)
        .add_property("data", extractEventData)
        ;
        
        register_ptr_to_python<boost::shared_ptr<Event>>();
        
        def("getcodec", getcodec);
        def("get_reverse_codec", get_reverse_codec);
        def("getvar", getvar);
        def("setvar", setvar);
        
        def("register_event_callback", register_event_callback);
        def("register_event_callback", register_event_callback_for_name);
        def("register_event_callback", register_event_callback_for_code);
        def("unregister_event_callbacks", unregister_event_callbacks);
        
        return module.ptr();
    } catch (const boost::python::error_already_set &) {
        // Python error already set.  Do nothing.
    } catch (const std::exception &e) {
        PyErr_Format(PyExc_RuntimeError,
                     "Error while initializing Python mworkscore module: %s",
                     e.what());
    } catch (...) {
        PyErr_SetString(PyExc_RuntimeError, "Unknown error while initializing Python mworkscore module");
    }
    
    return nullptr;
}


PyObject * getGlobalsDict() {
    static PyObject *globalsDict = nullptr;
    
    //
    // Perform one-time initialization of Python environment
    //
    static std::once_flag initFlag;
    std::call_once(initFlag, []() {
        //
        // Set module search path
        //
        {
            auto zipURL = cf::ObjectPtr<CFURLRef>::owned(CFBundleCopyResourceURL(getBundle().get(),
                                                                                 CFSTR("python" MW_PYTHON_3_VERSION_MAJOR MW_PYTHON_3_VERSION_MINOR),
                                                                                 CFSTR("zip"),
                                                                                 nullptr));
            if (!zipURL) {
                throw SimpleException(M_PLUGIN_MESSAGE_DOMAIN, "Cannot obtain path to Python library file");
            }
            auto decodedPath = Py_DecodeLocale(getPathForURL(zipURL).c_str(), nullptr);
            if (!decodedPath) {
                throw SimpleException(M_PLUGIN_MESSAGE_DOMAIN, "Cannot decode Python module search path");
            }
            Py_SetPath(decodedPath);
            PyMem_RawFree(decodedPath);
        }
        
        //
        // Register mworkscore module
        //
        if (-1 == PyImport_AppendInittab(mworkscoreModule.m_name, &init_mworkscore)) {
            throw PythonException("Unable to register Python mworkscore module");
        }
        
        //
        // Initialize Python interpreter
        //
        Py_NoSiteFlag = 1;  // Don't import the "site" module
        Py_InitializeEx(0);
        PyEval_InitThreads();
        BOOST_SCOPE_EXIT(void) {
            // PyEval_InitThreads acquires the GIL, so we must release it on exit
            PyEval_ReleaseThread(PyThreadState_Get());
        } BOOST_SCOPE_EXIT_END
        
        //
        // Obtain __main__ module's dict and store a reference in globalsDict
        //
        PyObject *mainModule = PyImport_AddModule("__main__");  // Borrowed ref
        if (!mainModule) {
            throw PythonException("Unable to import Python __main__ module");
        }
        globalsDict = PyModule_GetDict(mainModule);  // Borrowed ref, never fails
        Py_INCREF(globalsDict);  // Upgrade to owned ref
        
        //
        // Import mworks_python_config.  (This adds no names to globalsDict.)
        //
        PyObject *result = PyRun_String("from mworks_python_config import *", Py_single_input, globalsDict, globalsDict);
        if (!result) {
            throw PythonException("Unable to import mworks_python_config");
        }
        Py_DECREF(result);
        
        //
        // Import NumPy C API
        //
        if (_import_array() < 0) {
            throw PythonException("Unable to import NumPy C API");
        }
        
        //
        // Import mworkscore contents into __main__
        //
        result = PyRun_String("from mworkscore import *", Py_single_input, globalsDict, globalsDict);
        if (!result) {
            throw PythonException("Unable to import mworkscore contents into Python __main__ module");
        }
        Py_DECREF(result);
    });
    
    return globalsDict;
}


PyCodeObject * compile(const boost::filesystem::path &path) {
    const auto filename = path.string();
    
    std::FILE *fp = std::fopen(filename.c_str(), "r");
    if (!fp) {
        throw SimpleException(M_FILE_MESSAGE_DOMAIN, "Unable to open Python file", strerror(errno));
    }
    BOOST_SCOPE_EXIT(fp) {
        std::fclose(fp);
    } BOOST_SCOPE_EXIT_END
    
    ScopedGILAcquire sga;
    
    struct _node *node = PyParser_SimpleParseFile(fp, filename.c_str(), Py_file_input);
    BOOST_SCOPE_EXIT(node) {
        PyNode_Free(node);
    } BOOST_SCOPE_EXIT_END
    
    PyCodeObject *codeObject = nullptr;
    if (!node ||
        !(codeObject = PyNode_Compile(node, filename.c_str())))
    {
        throw PythonException("Python compilation failed");
    }
    
    return codeObject;
}


PyCodeObject * compile(const std::string &code, bool isExpr) {
    ScopedGILAcquire sga;
    
    struct _node *node = PyParser_SimpleParseString(code.c_str(), (isExpr ? Py_eval_input : Py_file_input));
    BOOST_SCOPE_EXIT(node) {
        PyNode_Free(node);
    } BOOST_SCOPE_EXIT_END
    
    PyCodeObject *codeObject = nullptr;
    if (!node ||
        !(codeObject = PyNode_Compile(node, "<string>")))
    {
        throw PythonException("Python compilation failed");
    }
    
    return codeObject;
}


class EvalState : boost::noncopyable {
    
public:
    EvalState() :
        //
        // Save the current working directory
        //
        cwdfd(open(".", O_RDONLY))
    {
        if (-1 == cwdfd) {
            merror(M_PLUGIN_MESSAGE_DOMAIN, "Unable to open current working directory: %s", strerror(errno));
        }
        
        //
        // Change the current working directory to the experiment's working path, so that Python code can
        // use relative paths to resource files
        //
        if (auto experiment = GlobalCurrentExperiment) {
            auto &workingPath = experiment->getWorkingPath();
            if (!(workingPath.empty())) {
                if (-1 == chdir(workingPath.c_str())) {
                    merror(M_PLUGIN_MESSAGE_DOMAIN, "Unable to change directory: %s", strerror(errno));
                }
            }
        }
    }
    
    ~EvalState() {
        //
        // Restore the current working directory
        //
        if (-1 != cwdfd) {
            (void)fchdir(cwdfd);
            (void)close(cwdfd);
        }
    }
    
private:
    ScopedGILAcquire sga;
    const int cwdfd;
    
};


END_NAMESPACE()


PythonEvaluator::PythonEvaluator(const boost::filesystem::path &filePath) :
    globalsDict(getGlobalsDict()),
    codeObject(compile(filePath))
{ }


PythonEvaluator::PythonEvaluator(const std::string &code, bool isExpr) :
    globalsDict(getGlobalsDict()),
    codeObject(compile(code, isExpr))
{ }


PythonEvaluator::~PythonEvaluator() {
    ScopedGILAcquire sga;
    Py_DECREF(codeObject);
}


bool PythonEvaluator::exec() {
    EvalState es;
    if (PyObject *result = eval()) {
        Py_DECREF(result);
        return true;
    }
    PythonException::logError("Python execution failed");
    return false;
}


bool PythonEvaluator::eval(Datum &result) {
    EvalState es;
    try {
        result = convert_python_to_datum(manageNewRef(eval()));
        return true;
    } catch (const boost::python::error_already_set &) {
        PythonException::logError("Python evaluation failed");
        return false;
    }
}


bool PythonEvaluator::call(Datum &result, ArgIter first, ArgIter last) {
    EvalState es;
    try {
        auto callable = manageNewRef(eval());
        
        const Py_ssize_t numArgs = last - first;
        auto args = manageNewRef(PyTuple_New(numArgs));
        for (Py_ssize_t argNum = 0; argNum < numArgs; argNum++) {
            auto arg = convert_datum_to_python(*(first + argNum));
            // PyTuple_SetItem "steals" the item reference, so we need to INCREF it
            Py_INCREF(arg.ptr());
            if (PyTuple_SetItem(args.ptr(), argNum, arg.ptr())) {
                throw_error_already_set();
            }
        }
        
        result = convert_python_to_datum(manageNewRef(PyObject_CallObject(callable.ptr(), args.ptr())));
        return true;
    } catch (const boost::python::error_already_set &) {
        PythonException::logError("Python call failed");
        return false;
    }
}


inline PyObject * PythonEvaluator::eval() {
    return PyEval_EvalCode(reinterpret_cast<PyObject *>(codeObject), globalsDict, globalsDict);
}


END_NAMESPACE_MW





















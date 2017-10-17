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


void init_mworkscore() {
    auto module = manageBorrowedRef( Py_InitModule("mworkscore", nullptr) );
    boost::python::scope moduleScope(module);
    
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
}


PyObject * getGlobalsDict() {
    static PyObject *globalsDict = nullptr;
    
    //
    // Perform one-time initialization of Python environment
    //
    static std::once_flag initFlag;
    std::call_once(initFlag, []() {
        //
        // Initialize Python interpreter
        //
        Py_InitializeEx(0);
        PyEval_InitThreads();
        BOOST_SCOPE_EXIT(void) {
            // PyEval_InitThreads acquires the GIL, so we must release it on exit
            PyEval_ReleaseThread(PyThreadState_Get());
        } BOOST_SCOPE_EXIT_END
        
        //
        // Import NumPy C API
        //
        if (_import_array() < 0) {
            throw PythonException("Unable to import NumPy C API");
        }
        
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
        // Create mworkscore module
        //
        try {
            init_mworkscore();
        } catch (const boost::python::error_already_set &) {
            throw PythonException("Unable to create Python mworkscore module");
        }
        
        //
        // Import mworkscore contents into __main__
        //
        PyObject *result = PyRun_String("from mworkscore import *", Py_single_input, globalsDict, globalsDict);
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


PyCodeObject * compile(const std::string &code) {
    ScopedGILAcquire sga;
    
    struct _node *node = PyParser_SimpleParseString(code.c_str(), Py_file_input);
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


END_NAMESPACE()


PythonEvaluator::PythonEvaluator(const boost::filesystem::path &filePath) :
    globalsDict(getGlobalsDict()),
    codeObject(compile(filePath))
{ }


PythonEvaluator::PythonEvaluator(const std::string &code) :
    globalsDict(getGlobalsDict()),
    codeObject(compile(code))
{ }


PythonEvaluator::~PythonEvaluator() {
    ScopedGILAcquire sga;
    Py_DECREF(codeObject);
}


bool PythonEvaluator::eval() {
    //
    // Save and (on exit) restore the current working directory
    //
    auto cwdfd = open(".", O_RDONLY);
    if (-1 == cwdfd) {
        merror(M_PLUGIN_MESSAGE_DOMAIN, "Unable to open current working directory: %s", strerror(errno));
    }
    BOOST_SCOPE_EXIT( &cwdfd ) {
        if (-1 != cwdfd) {
            (void)fchdir(cwdfd);
            (void)close(cwdfd);
        }
    } BOOST_SCOPE_EXIT_END
    
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
    
    ScopedGILAcquire sga;
    
    PyObject *result = PyEval_EvalCode(codeObject, globalsDict, globalsDict);
    if (result) {
        Py_DECREF(result);
        return true;
    }
    
    PythonException::logError("Python execution failed");
    return false;
}


END_NAMESPACE_MW





















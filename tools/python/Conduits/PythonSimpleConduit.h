/*
 *  PythonSimpleConduit.h
 *
 *  Created by David Cox on 10/2/08.
 *  Copyright 2008 The Rowland Institute at Harvard. All rights reserved.
 *
 */

#ifndef PYTHON_SIMPLE_CONDUIT_H_
#define PYTHON_SIMPLE_CONDUIT_H_

#include "PythonEvent.hpp"


BEGIN_NAMESPACE_MW_PYTHON


class PythonIPCConduit : public ExtensionType<PythonIPCConduit>, boost::noncopyable {

private:
    boost::shared_ptr<CodecAwareConduit> conduit;
    bool initialized = false;
    
    void requireValidConduit() const {
        if (!conduit || !initialized) {
            throw SimpleException("Conduit is not initialized");
        }
    }
    
public:
    ~PythonIPCConduit() {
        if (conduit && initialized) {
            ScopedGILRelease sgr;
            conduit->finalize();
        }
    }
    
    void init(const std::string &resource_name, bool correct_incoming_timestamps, int event_transport_type) {
#if defined(Py_LIMITED_API) && Py_LIMITED_API+0 <= 0x03060000
        // We'll be calling Python code in non-Python background threads, so ensure that the
        // GIL is initialized.  (This is necessary only in Python 3.6 and earlier.  PyEval_InitThreads
        // is deprecated as of Python 3.9 and will be removed in 3.11.)
        PyEval_InitThreads();
#endif
        
        // Need to hold the GIL until *after* we call PyEval_InitThreads
        ScopedGILRelease sgr;
        
        // if there's no clock defined, create the core infrastructure
        if (!Clock::instance(false)) {
            StandardServerCoreBuilder core_builder;
            CoreBuilderForeman::constructCoreStandardOrder(&core_builder);
        }
        
        // Finalize any existing conduit
        if (conduit && initialized) {
            conduit->finalize();
            initialized = false;
        }
        
        auto transport = boost::make_shared<IPCEventTransport>(static_cast<EventTransport::event_transport_type>(event_transport_type),
                                                               EventTransport::bidirectional_event_transport,
                                                               resource_name);
        conduit = boost::make_shared<CodecAwareConduit>(transport, correct_incoming_timestamps);
    }
    
    bool isInitialized() const {
        return initialized;
    }
    
    bool initialize() {
        if (conduit && !initialized) {
            ScopedGILRelease sgr;
            initialized = conduit->initialize();
        }
        return initialized;
    }
    
    void registerCallbackForCode(int code, const ObjectPtr &function_object) {
        requireValidConduit();
        
        PythonEvent::Callback cb(function_object);
        
        // Need to hold the GIL until *after* we create the PythonEvent::Callback, since doing so
        // involves an implicit Py_INCREF
        ScopedGILRelease sgr;
        
        conduit->registerCallback(code, cb);
    }

    void registerCallbackForName(const std::string &event_name, const ObjectPtr &function_object) {
        requireValidConduit();
        
        PythonEvent::Callback cb(function_object);
        
        // Need to hold the GIL until *after* we create the PythonEvent::Callback, since doing so
        // involves an implicit Py_INCREF
        ScopedGILRelease sgr;
        
        conduit->registerCallbackByName(event_name, cb);
    }
    
    
    void registerLocalEventCode(int code, const std::string &event_name) {
        requireValidConduit();
        
        ScopedGILRelease sgr;
        conduit->registerLocalEventCode(code, event_name);
    } 
    
    auto getCodec() const {
        requireValidConduit();
        
        std::map<int, std::string> codec;
        {
            ScopedGILRelease sgr;
            codec = conduit->getRemoteCodec();
        }
        return codec;
    }
    
    auto getReverseCodec() const {
        requireValidConduit();
        
        std::map<std::string, int> reverseCodec;
        {
            ScopedGILRelease sgr;
            reverseCodec = conduit->getRemoteReverseCodec();
        }
        return reverseCodec;
    }
    
    void finalize(){
        requireValidConduit();
        
        ScopedGILRelease sgr;
        conduit->finalize();
        initialized = false;
    }
    
    void sendData(int code, const Datum &data) {
        requireValidConduit();
        
        ScopedGILRelease sgr;
        conduit->sendData(code, data);
    }

};


END_NAMESPACE_MW_PYTHON


#endif

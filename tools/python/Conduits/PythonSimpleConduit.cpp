//
//  PythonSimpleConduit.cpp
//  Extension Module
//
//  Created by Christopher Stawarz on 10/16/19.
//  Copyright © 2019 MWorks Project. All rights reserved.
//

#include "PythonSimpleConduit.h"

#import <AppKit/AppKit.h>


BEGIN_NAMESPACE_MW_PYTHON


PythonIPCConduit::~PythonIPCConduit() {
    if (conduit && initialized) {
        ScopedGILRelease sgr;
        conduit->finalize();
    }
}


void PythonIPCConduit::init(const std::string &resource_name,
                            bool correct_incoming_timestamps,
                            bool cache_incoming_data,
                            int event_transport_type)
{
#if defined(Py_LIMITED_API) && Py_LIMITED_API+0 <= 0x03060000
    // We'll be calling Python code in non-Python background threads, so ensure that the
    // GIL is initialized.  (This is necessary only in Python 3.6 and earlier.  PyEval_InitThreads
    // is deprecated as of Python 3.9 and will be removed in 3.11.)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    PyEval_InitThreads();
#pragma clang diagnostic pop
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
    
    auto transport = boost::make_shared<ZeroMQIPCEventTransport>(static_cast<EventTransport::event_transport_type>(event_transport_type),
                                                                 resource_name);
    conduit = boost::make_shared<CodecAwareConduit>(transport, correct_incoming_timestamps);
    
    cacheIncomingData = cache_incoming_data;
    lock_guard lock(incomingDataCacheMutex);
    incomingDataCache.clear();
}


bool PythonIPCConduit::isInitialized() const {
    return initialized;
}


bool PythonIPCConduit::initialize() {
    if (conduit && !initialized) {
        ScopedGILRelease sgr;
        initialized = conduit->initialize();
        if (initialized && cacheIncomingData) {
            conduit->registerCallbackForAllEvents([this](boost::shared_ptr<Event> evt) {
                lock_guard lock(incomingDataCacheMutex);
                incomingDataCache[evt->getEventCode()] = evt->getData();
            });
        }
    }
    return initialized;
}


void PythonIPCConduit::finalize() {
    requireValidConduit();
    
    ScopedGILRelease sgr;
    
    conduit->finalize();
    initialized = false;
    
    lock_guard lock(incomingDataCacheMutex);
    incomingDataCache.clear();
}


void PythonIPCConduit::registerCallbackForAllEvents(const ObjectPtr &function_object) {
    requireValidConduit();
    
    PythonEvent::Callback cb(function_object);
    
    // Need to hold the GIL until *after* we create the PythonEvent::Callback, since doing so
    // involves an implicit Py_INCREF
    ScopedGILRelease sgr;
    
    conduit->registerCallbackForAllEvents(cb);
}


void PythonIPCConduit::registerCallbackForCode(int code, const ObjectPtr &function_object) {
    requireValidConduit();
    
    PythonEvent::Callback cb(function_object);
    
    // Need to hold the GIL until *after* we create the PythonEvent::Callback, since doing so
    // involves an implicit Py_INCREF
    ScopedGILRelease sgr;
    
    conduit->registerCallbackByCode(code, cb);
}


void PythonIPCConduit::registerCallbackForName(const std::string &event_name, const ObjectPtr &function_object) {
    requireValidConduit();
    
    PythonEvent::Callback cb(function_object);
    
    // Need to hold the GIL until *after* we create the PythonEvent::Callback, since doing so
    // involves an implicit Py_INCREF
    ScopedGILRelease sgr;
    
    conduit->registerCallbackByName(event_name, cb);
}


void PythonIPCConduit::registerLocalEventCode(int code, const std::string &event_name) {
    requireValidConduit();
    
    ScopedGILRelease sgr;
    conduit->registerLocalEventCode(code, event_name);
}


std::map<int, std::string> PythonIPCConduit::getCodec() const {
    requireValidConduit();
    
    std::map<int, std::string> codec;
    {
        ScopedGILRelease sgr;
        codec = conduit->getRemoteCodec();
    }
    return codec;
}


std::map<std::string, int> PythonIPCConduit::getReverseCodec() const {
    requireValidConduit();
    
    std::map<std::string, int> reverseCodec;
    {
        ScopedGILRelease sgr;
        reverseCodec = conduit->getRemoteReverseCodec();
    }
    return reverseCodec;
}


void PythonIPCConduit::sendData(int code, const Datum &data) {
    requireValidConduit();
    
    ScopedGILRelease sgr;
    conduit->sendData(code, data);
}


bool PythonIPCConduit::hasCachedDataForCode(int code) const {
    lock_guard lock(incomingDataCacheMutex);
    return incomingDataCache.count(code);
}


Datum PythonIPCConduit::getCachedDataForCode(int code) const {
    lock_guard lock(incomingDataCacheMutex);
    Datum data;
    auto iter = incomingDataCache.find(code);
    if (iter != incomingDataCache.end()) {
        data = iter->second;
    }
    return data;
}


static void stopMainLoop() {
    dispatch_async(dispatch_get_main_queue(), ^{
        [NSApplication.sharedApplication stop:nil];
        // Per the docs, "stop:" "sets a flag that the application checks only after it finishes
        // dispatching an actual event object", so we send a dummy event to ensure that the main
        // loop stops ASAP
        [NSApplication.sharedApplication postEvent:[NSEvent otherEventWithType:NSEventTypeApplicationDefined
                                                                      location:NSZeroPoint
                                                                 modifierFlags:0
                                                                     timestamp:0.0
                                                                  windowNumber:0
                                                                       context:nil
                                                                       subtype:0
                                                                         data1:0
                                                                         data2:0]
                                           atStart:YES];
    });
}


template<>
PyMethodDef ExtensionType<PythonIPCConduit>::methods[] = {
    MethodDef<&PythonIPCConduit::initialize>("initialize"),
    MethodDef<&PythonIPCConduit::finalize>("finalize"),
    MethodDef<&PythonIPCConduit::registerCallbackForAllEvents>("register_callback_for_all_events"),
    MethodDef<&PythonIPCConduit::registerCallbackForCode>("register_callback_for_code"),
    MethodDef<&PythonIPCConduit::registerCallbackForName>("register_callback_for_name"),
    MethodDef<&PythonIPCConduit::registerLocalEventCode>("register_local_event_code"),
    MethodDef<&PythonIPCConduit::sendData>("send_data"),
    MethodDef<&PythonIPCConduit::hasCachedDataForCode>("_has_cached_data_for_code"),
    MethodDef<&PythonIPCConduit::getCachedDataForCode>("_get_cached_data_for_code"),
    MethodDef<stopMainLoop>("_stop_main_loop"),
    { }  // Sentinel
};


template<>
PyGetSetDef ExtensionType<PythonIPCConduit>::getset[] = {
    GetSetDef<&PythonIPCConduit::isInitialized>("initialized"),
    GetSetDef<&PythonIPCConduit::getCodec>("codec"),
    GetSetDef<&PythonIPCConduit::getReverseCodec>("reverse_codec"),
    { }  // Sentinel
};


END_NAMESPACE_MW_PYTHON

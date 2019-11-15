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
    MethodDef<&PythonIPCConduit::registerCallbackForCode>("register_callback_for_code"),
    MethodDef<&PythonIPCConduit::registerCallbackForName>("register_callback_for_name"),
    MethodDef<&PythonIPCConduit::registerLocalEventCode>("register_local_event_code"),
    MethodDef<&PythonIPCConduit::finalize>("finalize"),
    MethodDef<&PythonIPCConduit::sendData>("send_data"),
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

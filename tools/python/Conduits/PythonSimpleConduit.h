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
    
public:
    ~PythonIPCConduit();
    
    void init(const std::string &resource_name, bool correct_incoming_timestamps, int event_transport_type);
    
    bool isInitialized() const;
    bool initialize();
    void finalize();
    
    void registerCallbackForCode(int code, const ObjectPtr &function_object);
    void registerCallbackForName(const std::string &event_name, const ObjectPtr &function_object);
    void registerLocalEventCode(int code, const std::string &event_name);
    
    std::map<int, std::string> getCodec() const;
    std::map<std::string, int> getReverseCodec() const;
    
    void sendData(int code, const Datum &data);
    
private:
    void requireValidConduit() const {
        if (!conduit || !initialized) {
            throw SimpleException("Conduit is not initialized");
        }
    }
    
    boost::shared_ptr<CodecAwareConduit> conduit;
    bool initialized = false;
    
};


END_NAMESPACE_MW_PYTHON


#endif

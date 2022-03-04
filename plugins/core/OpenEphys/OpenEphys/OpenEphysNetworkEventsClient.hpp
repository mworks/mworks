//
//  OpenEphysNetworkEventsClient.hpp
//  OpenEphys
//
//  Created by Christopher Stawarz on 5/24/16.
//  Copyright © 2016 The MWorks Project. All rights reserved.
//

#ifndef OpenEphysNetworkEventsClient_hpp
#define OpenEphysNetworkEventsClient_hpp


#include "OpenEphysBase.hpp"


BEGIN_NAMESPACE_MW


class OpenEphysNetworkEventsClient : public OpenEphysBase {
    
public:
    static const std::string REQUEST;
    static const std::string RESPONSE;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit OpenEphysNetworkEventsClient(const ParameterValueMap &parameters);
    ~OpenEphysNetworkEventsClient();
    
    bool initialize() override;
    
private:
    void sendRequest(const std::string &req);
    
    const VariablePtr request;
    const VariablePtr response;
    
};


END_NAMESPACE_MW


#endif /* OpenEphysNetworkEventsClient_hpp */




























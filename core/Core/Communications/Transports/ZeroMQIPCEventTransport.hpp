//
//  ZeroMQIPCEventTransport.hpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 6/8/21.
//

#ifndef ZeroMQIPCEventTransport_hpp
#define ZeroMQIPCEventTransport_hpp

#include "EventTransport.h"
#include "ZeroMQSocketPair.hpp"


BEGIN_NAMESPACE_MW


class ZeroMQIPCEventTransport : public EventTransport {
    
public:
    ZeroMQIPCEventTransport(event_transport_type type, const std::string &resourceName);
    ~ZeroMQIPCEventTransport();
    
    void sendEvent(const boost::shared_ptr<Event> &event) override;
    boost::shared_ptr<Event> receiveEventAsynchronous() override;
    
private:
    std::unique_ptr<ZeroMQSocketPair> socketPair;
    
};


END_NAMESPACE_MW


#endif /* ZeroMQIPCEventTransport_hpp */

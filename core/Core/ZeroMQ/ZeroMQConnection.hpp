//
//  ZeroMQConnection.hpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 7/29/16.
//
//

#ifndef ZeroMQConnection_hpp
#define ZeroMQConnection_hpp

#include <thread>

#include "EventBuffer.h"
#include "ZeroMQSocket.hpp"


BEGIN_NAMESPACE_MW


class ZeroMQConnection {
    
public:
    ZeroMQConnection(ZeroMQSocket &socket, const boost::shared_ptr<EventBuffer> &eventBuffer);
    virtual ~ZeroMQConnection();
    
    void start();
    void stop();
    
    bool isRunning() const {
        return running;
    }
    
protected:
    static constexpr int receiveTimeoutMS = 500;
    static constexpr int sendTimeoutMS = 2000;
    
    ZeroMQSocket &socket;
    const boost::shared_ptr<EventBuffer> eventBuffer;
    static_assert(ATOMIC_BOOL_LOCK_FREE == 2, "std::atomic_bool is not always lock-free");
    std::atomic_bool running;
    
private:
    virtual void handleEvents() = 0;
    
    std::thread eventHandlerThread;
    
};


class ZeroMQIncomingConnection : public ZeroMQConnection {
    
public:
    using ZeroMQConnection::ZeroMQConnection;
    
private:
    void handleEvents() override;
    
};


class ZeroMQOutgoingConnection : public ZeroMQConnection {
    
public:
    using ZeroMQConnection::ZeroMQConnection;
    
private:
    void handleEvents() override;
    
};


END_NAMESPACE_MW


#endif /* ZeroMQConnection_hpp */



























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


class ZeroMQConnectionBase {
    
public:
    ZeroMQConnectionBase(ZeroMQSocket &socket, const boost::shared_ptr<EventBuffer> &eventBuffer) :
        socket(socket),
        eventBuffer(eventBuffer),
        running(false)
    { }
    
protected:
    static constexpr int receiveTimeoutMS = 500;
    static constexpr int sendTimeoutMS = 2000;
    
    ZeroMQSocket &socket;
    const boost::shared_ptr<EventBuffer> eventBuffer;
    static_assert(ATOMIC_BOOL_LOCK_FREE == 2, "std::atomic_bool is not always lock-free");
    std::atomic_bool running;
    
};


class ZeroMQIncomingConnectionBase : public ZeroMQConnectionBase {
    
public:
    ZeroMQIncomingConnectionBase(ZeroMQSocket &socket, const boost::shared_ptr<EventBuffer> &eventBuffer);
    
protected:
    std::thread startEventHandlerThread();
    
private:
    void handleNextEvent();
    
};


class ZeroMQOutgoingConnectionBase : public ZeroMQConnectionBase {
    
public:
    ZeroMQOutgoingConnectionBase(ZeroMQSocket &socket, const boost::shared_ptr<EventBuffer> &eventBuffer);
    
protected:
    std::thread startEventHandlerThread();
    
private:
    void handleEvents();
    
    std::unique_ptr<EventBufferReader> eventBufferReader;
    
};


template <typename Base>
class ZeroMQConnection : private Base {
    
public:
    using Base::Base;
    
    ~ZeroMQConnection() {
        stop();
    }
    
    void start() {
        if (!eventHandlerThread.joinable()) {
            Base::running = true;
            eventHandlerThread = Base::startEventHandlerThread();
        }
    }
    
    void stop() {
        if (eventHandlerThread.joinable()) {
            Base::running = false;
            eventHandlerThread.join();
        }
    }
    
    bool isRunning() const {
        return Base::running;
    }
    
private:
    std::thread eventHandlerThread;
    
};


using ZeroMQIncomingConnection = ZeroMQConnection<ZeroMQIncomingConnectionBase>;
using ZeroMQOutgoingConnection = ZeroMQConnection<ZeroMQOutgoingConnectionBase>;


END_NAMESPACE_MW


#endif /* ZeroMQConnection_hpp */



























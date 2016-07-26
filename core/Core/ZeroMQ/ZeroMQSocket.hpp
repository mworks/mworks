//
//  ZeroMQSocket.hpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 7/26/16.
//  Copyright © 2016 The MWorks Project. All rights reserved.
//

#ifndef ZeroMQSocket_hpp
#define ZeroMQSocket_hpp

#include <string>

#include <boost/noncopyable.hpp>
#include <zmq.h>

#include "Event.h"


BEGIN_NAMESPACE_MW


class ZeroMQSocket : boost::noncopyable {
    
public:
    enum class Result {
        ok,
        error,
        timeout
    };
    
    class Message : boost::noncopyable {
    public:
        Message();
        ~Message();
        Result recv(ZeroMQSocket &socket, int flags = 0);
        const void * getData() const;
        std::size_t getSize() const;
        bool isComplete() const;
    private:
        zmq_msg_t msg;
    };
    
    explicit ZeroMQSocket(int type);
    ~ZeroMQSocket();
    
    bool setOption(int name, const void *value, std::size_t valueSize);
    bool setOption(int name, int value) { return setOption(name, &value, sizeof(value)); }
    bool setOption(int name, const std::string &value) { return setOption(name, value.data(), value.size()); }
    
    bool bind(const std::string &endpoint);
    bool unbind(const std::string &endpoint);
    
    bool connect(const std::string &endpoint);
    bool disconnect(const std::string &endpoint);
    
    Result send(const void *data, std::size_t dataSize, int flags = 0);
    
    Result send(const boost::shared_ptr<Event> &event);
    Result recv(boost::shared_ptr<Event> &event);
    
private:
    void * const socket;
    
};


END_NAMESPACE_MW


#endif /* ZeroMQSocket_hpp */



























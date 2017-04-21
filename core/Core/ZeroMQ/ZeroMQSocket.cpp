//
//  ZeroMQSocket.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 7/26/16.
//  Copyright © 2016 The MWorks Project. All rights reserved.
//

#include "ZeroMQSocket.hpp"

#include <boost/endian/arithmetic.hpp>

#include "MessagePackAdaptors.hpp"
#include "Utilities.h"


BEGIN_NAMESPACE_MW


BEGIN_NAMESPACE()


void logError(const std::string &message) {
    merror(M_NETWORK_MESSAGE_DOMAIN, "%s: %s", message.c_str(), zmq_strerror(errno));
}


struct ContextDeleter {
    void operator()(void *context) {
        do {
            if (-1 != zmq_ctx_term(context)) {
                return;
            }
            if (errno != EINTR) {
                break;
            }
        } while (true);
        
        logError("Cannot terminate ZeroMQ context");
    }
};


void * getContext() {
    static const std::unique_ptr<void, ContextDeleter> context(zmq_ctx_new());
    if (!context) {
        throw SimpleException(M_NETWORK_MESSAGE_DOMAIN, "Cannot create ZeroMQ context");
    }
    return context.get();
}


END_NAMESPACE()


ZeroMQSocket::Message::Message() {
    (void)zmq_msg_init(&msg);  // Never fails
}


ZeroMQSocket::Message::~Message() {
    if (-1 == zmq_msg_close(&msg)) {
        logError("Cannot close ZeroMQ message");
    }
}


auto ZeroMQSocket::Message::recv(ZeroMQSocket &socket, int flags) -> Result {
    do {
        if (-1 != zmq_msg_recv(&msg, socket.socket, flags)) {
            return Result::ok;
        }
        if (errno != EINTR) {
            break;
        }
    } while (true);
    
    if (errno == EAGAIN) {
        return Result::timeout;
    }
    
    logError("Cannot receive on ZeroMQ socket");
    return Result::error;
}


const void * ZeroMQSocket::Message::getData() const {
    return zmq_msg_data(const_cast<zmq_msg_t *>(&msg));
}


std::size_t ZeroMQSocket::Message::getSize() const {
    return zmq_msg_size(const_cast<zmq_msg_t *>(&msg));
}


bool ZeroMQSocket::Message::isComplete() const {
    return !zmq_msg_more(const_cast<zmq_msg_t *>(&msg));
}


ZeroMQSocket::ZeroMQSocket(int type) :
    socket(zmq_socket(getContext(), type))
{
    if (!socket) {
        throw SimpleException(M_NETWORK_MESSAGE_DOMAIN, "Cannot create ZeroMQ socket", zmq_strerror(errno));
    }
    if (!(setOption(ZMQ_IPV6, 1) && setOption(ZMQ_LINGER, 0))) {
        throw SimpleException(M_NETWORK_MESSAGE_DOMAIN, "Cannot configure ZeroMQ socket");
    }
}


ZeroMQSocket::~ZeroMQSocket() {
    if (-1 == zmq_close(socket)) {
        logError("Cannot close ZeroMQ socket");
    }
}


bool ZeroMQSocket::getLastEndpoint(std::string &endpoint) const {
    do {
        std::array<char, 1024> value;
        std::size_t valueSize = value.size();
        if (-1 != zmq_getsockopt(socket, ZMQ_LAST_ENDPOINT, value.data(), &valueSize)) {
            endpoint.assign(value.data(), valueSize - 1);
            return true;
        }
        if (errno != EINTR) {
            break;
        }
    } while (true);
    
    logError("Cannot retrieve last endpoint of ZeroMQ socket");
    return false;
}


bool ZeroMQSocket::setOption(int name, const void *value, std::size_t valueSize) {
    do {
        if (-1 != zmq_setsockopt(socket, name, value, valueSize)) {
            return true;
        }
        if (errno != EINTR) {
            break;
        }
    } while (true);
    
    logError("Cannot set option on ZeroMQ socket");
    return false;
}


bool ZeroMQSocket::bind(const std::string &endpoint) {
    if (-1 == zmq_bind(socket, endpoint.c_str())) {
        logError("Cannot bind ZeroMQ socket");
        return false;
    }
    return true;
}


bool ZeroMQSocket::unbind(const std::string &endpoint) {
    if (-1 == zmq_unbind(socket, endpoint.c_str())) {
        logError("Cannot unbind ZeroMQ socket");
        return false;
    }
    return true;
}


bool ZeroMQSocket::connect(const std::string &endpoint) {
    if (-1 == zmq_connect(socket, endpoint.c_str())) {
        logError("Cannot connect ZeroMQ socket");
        return false;
    }
    return true;
}


bool ZeroMQSocket::disconnect(const std::string &endpoint) {
    if (-1 == zmq_disconnect(socket, endpoint.c_str())) {
        logError("Cannot disconnect ZeroMQ socket");
        return false;
    }
    return true;
}


auto ZeroMQSocket::send(const void *data, std::size_t dataSize, int flags) -> Result {
    do {
        if (-1 != zmq_send(socket, data, dataSize, flags)) {
            return Result::ok;
        }
        if (errno != EINTR) {
            break;
        }
    } while (true);
    
    if (errno == EAGAIN) {
        return Result::timeout;
    }
    
    logError("Cannot send on ZeroMQ socket");
    return Result::error;
}


auto ZeroMQSocket::send(const boost::shared_ptr<Event> &event) -> Result {
    // Send code
    {
        int code = event->getEventCode();
        if (code < 0 || code > std::numeric_limits<std::uint16_t>::max()) {
            merror(M_NETWORK_MESSAGE_DOMAIN, "Cannot send event: event code (%d) is out of range", code);
            return Result::error;
        }
        boost::endian::big_uint16_t codeBytes = code;
        Result result;
        if (Result::ok != (result = send(&codeBytes, sizeof(codeBytes), ZMQ_SNDMORE))) {
            return result;
        }
    }
    
    // Send time and data
    {
        std::ostringstream buffer;
        msgpack::pack(buffer, event->getTime());
        msgpack::pack(buffer, event->getData());
        Result result;
        auto data = buffer.str();
        if (Result::ok != (result = send(data.data(), data.size()))) {
            return result;
        }
    }
    
    return Result::ok;
}


auto ZeroMQSocket::recv(boost::shared_ptr<Event> &event) -> Result {
    Message msg;
    int code = M_UNDEFINED_EVENT_CODE;
    MWTime time = 0;
    Datum data;
    
    // Receive code
    {
        Result result;
        if (Result::ok != (result = msg.recv(*this))) {
            return result;
        }
        boost::endian::big_uint16_t codeBytes;
        auto msgSize = msg.getSize();
        if (msgSize != sizeof(codeBytes)) {
            merror(M_NETWORK_MESSAGE_DOMAIN,
                   "Received invalid event: header has wrong size (expected %lu bytes, got %lu bytes)",
                   sizeof(codeBytes),
                   msgSize);
            return Result::error;
        }
        std::memcpy(&codeBytes, msg.getData(), msgSize);
        code = codeBytes;
    }
    
    // Receive time and data
    {
        if (msg.isComplete()) {
            merror(M_NETWORK_MESSAGE_DOMAIN, "Received invalid event: body is missing");
            return Result::error;
        }
        
        Result result;
        if (Result::ok != (result = msg.recv(*this))) {
            return result;
        }
        
        auto msgData = static_cast<const char *>(msg.getData());
        auto msgSize = msg.getSize();
        std::size_t offset = 0;
        msgpack::object_handle handle;
        try {
            
            // Extract time
            handle = msgpack::unpack(msgData, msgSize, offset);
            time = handle.get().as<MWTime>();
            
            // Extract data
            handle = msgpack::unpack(msgData, msgSize, offset);
            data = handle.get().as<Datum>();
            
        } catch (const std::exception &) {  // All msgpack exceptions derive from std::exception
            merror(M_NETWORK_MESSAGE_DOMAIN, "Received invalid event: body is invalid or corrupt");
            return Result::error;
        }
        
        if (offset < msgSize) {
            mwarning(M_NETWORK_MESSAGE_DOMAIN,
                     "Received event body contains %lu bytes of extra data that will be ignored",
                     msgSize - offset);
        }
    }
    
    event = boost::make_shared<Event>(code, time, data);
    
    return Result::ok;
}


END_NAMESPACE_MW



























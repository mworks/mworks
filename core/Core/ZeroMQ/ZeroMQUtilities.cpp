//
//  ZeroMQUtilities.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 8/3/16.
//
//

#include "ZeroMQUtilities.hpp"

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <boost/format.hpp>
#include <boost/scope_exit.hpp>

#include "Utilities.h"


BEGIN_NAMESPACE_MW
BEGIN_NAMESPACE(zeromq)


std::string formatTCPEndpoint(const std::string &hostname, int port) {
    return (boost::format("tcp://%1%:%2%") % hostname % port).str();
}


bool resolveHostname(const std::string &hostname, std::string &address) {
    const struct addrinfo hints = {
        AI_DEFAULT,
        PF_INET,
        SOCK_STREAM,
        IPPROTO_TCP,
        0,
        nullptr,
        nullptr,
        nullptr
    };
    
    struct addrinfo *ai = nullptr;
    BOOST_SCOPE_EXIT(&ai) {
        if (ai) {
            freeaddrinfo(ai);
        }
    } BOOST_SCOPE_EXIT_END
    
    std::array<char, NI_MAXHOST> host;
    int error;
    if (0 != (error = getaddrinfo(hostname.c_str(), nullptr, &hints, &ai)) ||
        // Convert the first returned address structure into numeric (text) form
        0 != (error = getnameinfo(ai->ai_addr, ai->ai_addrlen, host.data(), host.size(), nullptr, 0, NI_NUMERICHOST)))
    {
        merror(M_NETWORK_MESSAGE_DOMAIN, "Cannot resolve hostname \"%s\": %s", hostname.c_str(), gai_strerror(error));
        return false;
    }
    
    address = host.data();
    
    return true;
}


END_NAMESPACE(zeromq)
END_NAMESPACE_MW




























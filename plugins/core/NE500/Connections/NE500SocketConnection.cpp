//
//  NE500SocketConnection.cpp
//  NE500Plugin
//
//  Created by Christopher Stawarz on 3/29/16.
//
//

#include "NE500SocketConnection.hpp"

#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <fcntl.h>


BEGIN_NAMESPACE_MW


bool NE500SocketConnection::connect() {
    if (connected) {
        return true;
    }
    
    struct hostent *hostp;
    if (NULL == (hostp = gethostbyname(address.c_str()))) {
        merror(M_IODEVICE_MESSAGE_DOMAIN, "Host lookup failed: %s: %s", address.c_str(), hstrerror(h_errno));
        return false;
    }
    
    if ((s = socket(PF_INET, SOCK_STREAM, 6)) < 0) {
        merror(M_IODEVICE_MESSAGE_DOMAIN, "Socket creation failed: %s", strerror(errno));
        return false;
    }
    
    // Set socket to non-blocking
    int flags;
    if ((flags = fcntl(s, F_GETFL, 0)) < 0 ||
        fcntl(s, F_SETFL, flags | O_NONBLOCK) < 0)
    {
        merror(M_IODEVICE_MESSAGE_DOMAIN, "Cannot set socket to non-blocking: %s", strerror(errno));
        close(s);
        return false;
    }
    
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    memcpy(&(addr.sin_addr), hostp->h_addr, hostp->h_length);
    addr.sin_family = hostp->h_addrtype;
    addr.sin_port = htons((u_short)port);
    
    while (true) {
        int rc = ::connect(s, (struct sockaddr *) &addr, sizeof(addr));
        int connect_errno = errno;
        
        if (rc >= 0 || connect_errno == EISCONN) {
            break;
        }
        
        if (connect_errno != EALREADY &&
            connect_errno != EINPROGRESS &&
            connect_errno != EWOULDBLOCK)
        {
            merror(M_IODEVICE_MESSAGE_DOMAIN, "Cannot connect to %s: %s", address.c_str(), strerror(connect_errno));
            close(s);
            return false;
        }
        
        if (connect_errno == EINPROGRESS) {
            timeval timeout;
            timeout.tv_sec = 5;
            timeout.tv_usec = 0;
            fd_set fd_w;
            FD_ZERO(&fd_w);
            FD_SET(s, &fd_w);
            
            // 0 means it timed out out & no fds changed
            if (0 == select(FD_SETSIZE, NULL, &fd_w, NULL, &timeout)) {
                merror(M_IODEVICE_MESSAGE_DOMAIN, "Connection to NE500 device (%s) timed out", address.c_str());
                close(s);
                return false;
            }
        }
    }
    
    connected = true;
    return true;
}


bool NE500SocketConnection::disconnect() {
    if (connected) {
        if (close(s) < 0) {
            mwarning(M_IODEVICE_MESSAGE_DOMAIN,
                     "Cannot disconnect from NE500 device (%s): %s",
                     address.c_str(),
                     strerror(errno));
        }
        connected = false;
    }
    
    return true;
}


bool NE500SocketConnection::send(const std::string &command) {
    if (!connected) {
        return false;
    }
    
    if (::send(s, command.data(), command.size(), 0) < 0) {
        merror(M_IODEVICE_MESSAGE_DOMAIN,
               "Cannot send data to NE500 device (%s): %s",
               address.c_str(),
               strerror(errno));
        return false;
    }
    
    return true;
}


bool NE500SocketConnection::receive(std::string &response) {
    if (!connected) {
        return false;
    }
    
    std::array<char, 512> buffer;
    auto rc = ::recv(s, buffer.data(), buffer.size(), 0);
    
    if (rc > 0) {
        
        response.append(buffer.data(), rc);
        
    } else if (rc == 0) {
        
        merror(M_IODEVICE_MESSAGE_DOMAIN, "Connection to NE500 device (%s) was closed on remote end", address.c_str());
        disconnect();
        return false;
        
    } else if (errno != EWOULDBLOCK) {
        
        merror(M_IODEVICE_MESSAGE_DOMAIN,
               "Cannot receive data from NE500 device (%s): %s",
               address.c_str(),
               strerror(errno));
        return false;
        
    }
    
    return true;
}


END_NAMESPACE_MW




























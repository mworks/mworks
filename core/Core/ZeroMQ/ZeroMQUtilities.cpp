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
#include <net/if.h>
#include <ifaddrs.h>
#include <netdb.h>

#include <boost/format.hpp>
#include <boost/scope_exit.hpp>

#include "Utilities.h"


BEGIN_NAMESPACE_MW
BEGIN_NAMESPACE(zeromq)


std::string formatTCPEndpoint(const std::string &hostname, int port) {
    return (boost::format("tcp://%1%:%2%") % hostname % port).str();
}


bool getHostname(std::string &hostname) {
    struct ifaddrs *interfaces = nullptr;
    if (0 != getifaddrs(&interfaces)) {
        merror(M_NETWORK_MESSAGE_DOMAIN, "Cannot get network interface addresses: %s", strerror(errno));
    } else {
        BOOST_SCOPE_EXIT(&interfaces) {
            freeifaddrs(interfaces);
        } BOOST_SCOPE_EXIT_END
        
        //
        // We want the "most public" hostname or address, so we have to jump through some hoops.  First,
        // we classify each active IPv4 and IPv6 address as loopback, link local, private, or "default"
        // (meaning none of the first three classes).  Next, we sort the addresses by class, with
        // default first, private second, link local third, and loopback last.  Finally, we try to
        // convert each address in the sorted list to a name or numeric address and return the first one
        // that succeeds.
        //
        
        enum {
            DEFAULT,
            PRIVATE,
            LINKLOCAL,
            LOOPBACK
        };
        using AddressInfo = std::pair<struct sockaddr *, int>;
        std::vector<AddressInfo> candidateAddrs;
        
        for (auto ifa = interfaces; ifa; ifa = ifa->ifa_next) {
            if ((ifa->ifa_flags & IFF_UP) && ifa->ifa_addr) {
                auto addr = ifa->ifa_addr;
                auto addrType = DEFAULT;
                
                switch (addr->sa_family) {
                    case AF_INET: {
                        // s_addr is stored in network (i.e. big-endian) byte order, so we must convert it
                        // to host byte order before using it
                        auto s_addr = ntohl(reinterpret_cast<struct sockaddr_in *>(addr)->sin_addr.s_addr);
                        if (IN_LOOPBACK(s_addr)) {
                            addrType = LOOPBACK;
                        } else if (IN_LINKLOCAL(s_addr)) {
                            addrType = LINKLOCAL;
                        } else if (IN_PRIVATE(s_addr)) {
                            addrType = PRIVATE;
                        }
                        break;
                    }
                        
                    case AF_INET6: {
                        auto sin6_addr = &(reinterpret_cast<struct sockaddr_in6 *>(addr)->sin6_addr);
                        if (IN6_IS_ADDR_LOOPBACK(sin6_addr)) {
                            addrType = LOOPBACK;
                        } else if (IN6_IS_ADDR_LINKLOCAL(sin6_addr)) {
                            addrType = LINKLOCAL;
                        } else if (IN6_IS_ADDR_SITELOCAL(sin6_addr) || IN6_IS_ADDR_UNIQUE_LOCAL(sin6_addr)) {
                            addrType = PRIVATE;
                        }
                        break;
                    }
                        
                    default:
                        continue;
                }
                
                candidateAddrs.emplace_back(addr, addrType);
            }
        }
        
        // Use stable_sort to preserve the original ordering of addresses within each type
        std::stable_sort(candidateAddrs.begin(), candidateAddrs.end(), [](const AddressInfo &a, const AddressInfo &b) {
            return (a.second < b.second);
        });
        
        for (const auto &info : candidateAddrs) {
            auto &addr = info.first;
            std::array<char, NI_MAXHOST> host;
            auto error = getnameinfo(addr, addr->sa_len, host.data(), host.size(), nullptr, 0, 0);
            if (0 == error) {
                hostname = host.data();
                return true;
            }
            merror(M_NETWORK_MESSAGE_DOMAIN,
                   "Cannot convert socket address to host name: %s",
                   gai_strerror(error));
        }
    }
    
    merror(M_NETWORK_MESSAGE_DOMAIN, "Cannot determine system hostname");
    return false;
}


bool resolveHostname(const std::string &hostname, std::string &address) {
    const struct addrinfo hints = {
        AI_DEFAULT,
        PF_UNSPEC,
        SOCK_STREAM,
        IPPROTO_TCP,
        0,
        nullptr,
        nullptr,
        nullptr
    };
    
    struct addrinfo *addresses = nullptr;
    auto error = getaddrinfo(hostname.c_str(), nullptr, &hints, &addresses);
    if (0 != error) {
        merror(M_NETWORK_MESSAGE_DOMAIN, "Cannot resolve hostname \"%s\": %s", hostname.c_str(), gai_strerror(error));
        return false;
    }
    BOOST_SCOPE_EXIT(&addresses) {
        freeaddrinfo(addresses);
    } BOOST_SCOPE_EXIT_END
    
    // Convert the first returned IPv4 or IPv6 address structure into numeric (text) form
    for (auto ai = addresses; ai; ai = ai->ai_next) {
        if (ai->ai_family == PF_INET || ai->ai_family == PF_INET6) {
            std::array<char, NI_MAXHOST> host;
            error = getnameinfo(ai->ai_addr, ai->ai_addrlen, host.data(), host.size(), nullptr, 0, NI_NUMERICHOST);
            if (0 == error) {
                address = host.data();
                return true;
            }
            merror(M_NETWORK_MESSAGE_DOMAIN,
                   "Cannot convert socket address to numeric form: %s",
                   gai_strerror(error));
        }
    }
    
    merror(M_NETWORK_MESSAGE_DOMAIN, "Cannot convert hostname \"%s\" to a usable address", hostname.c_str());
    return false;
}


END_NAMESPACE(zeromq)
END_NAMESPACE_MW

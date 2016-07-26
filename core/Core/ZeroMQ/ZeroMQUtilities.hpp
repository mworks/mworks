//
//  ZeroMQUtilities.hpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 8/3/16.
//
//

#ifndef ZeroMQUtilities_hpp
#define ZeroMQUtilities_hpp

#include <string>

#include "MWorksMacros.h"


BEGIN_NAMESPACE_MW
BEGIN_NAMESPACE(zeromq)


std::string formatTCPEndpoint(const std::string &hostname, int port);
bool resolveHostname(const std::string &hostname, std::string &address);


END_NAMESPACE(zeromq)
END_NAMESPACE_MW


#endif /* ZeroMQUtilities_hpp */

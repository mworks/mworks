//
//  FileHash.hpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 10/9/24.
//

#ifndef FileHash_hpp
#define FileHash_hpp

#include <string>

#include "MWorksMacros.h"


BEGIN_NAMESPACE_MW


std::string computeFileHash(const std::string &filePath);


END_NAMESPACE_MW


#endif /* FileHash_hpp */

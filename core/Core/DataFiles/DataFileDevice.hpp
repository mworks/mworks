//
//  DataFileDevice.hpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 7/25/19.
//

#ifndef DataFileDevice_hpp
#define DataFileDevice_hpp

#include "IODevice.h"


BEGIN_NAMESPACE_MW


class DataFileDevice : public IODevice {
    
public:
    static const std::string FILENAME;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit DataFileDevice(const ParameterValueMap &parameters);
    
};


END_NAMESPACE_MW


#endif /* DataFileDevice_hpp */

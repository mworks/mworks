//
//  RCBLVDSDigitalInputChannel.hpp
//  RCBLVDS
//
//  Created by Christopher Stawarz on 9/25/17.
//  Copyright © 2017 The MWorks Project. All rights reserved.
//

#ifndef RCBLVDSDigitalInputChannel_hpp
#define RCBLVDSDigitalInputChannel_hpp


BEGIN_NAMESPACE_MW


class RCBLVDSDigitalInputChannel : public Component {
    
public:
    static const std::string CHANNEL;
    static const std::string VALUE;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit RCBLVDSDigitalInputChannel(const ParameterValueMap &parameters);
    
    const std::vector<std::size_t>& getChannelNumbers() const { return channelNumbers; }
    const VariablePtr& getValueVariable() const { return value; }
    
private:
    static std::vector<std::size_t> getChannelNumbers(const std::string &channel);
    
    const std::vector<std::size_t> channelNumbers;
    const VariablePtr value;

};


END_NAMESPACE_MW


#endif /* RCBLVDSDigitalInputChannel_hpp */





















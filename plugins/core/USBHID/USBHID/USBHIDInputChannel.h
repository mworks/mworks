//
//  USBHIDInputChannel.h
//  USBHID
//
//  Created by Christopher Stawarz on 4/4/13.
//  Copyright (c) 2013 The MWorks Project. All rights reserved.
//

#ifndef __USBHID__USBHIDInputChannel__
#define __USBHID__USBHIDInputChannel__


BEGIN_NAMESPACE_MW


class USBHIDInputChannel : public Component {
    
public:
    static const std::string USAGE_PAGE;
    static const std::string USAGE;
    static const std::string VALUE;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit USBHIDInputChannel(const ParameterValueMap &parameters);
    
    long getUsagePage() const { return usagePage; }
    long getUsage() const { return usage; }
    
    void postValue(CFIndex integerValue, MWTime time) const {
        value->setValue(integerValue, time);
    }
    
private:
    const long usagePage;
    const long usage;
    const VariablePtr value;
    
};


END_NAMESPACE_MW


#endif // !defined(__USBHID__USBHIDInputChannel__)

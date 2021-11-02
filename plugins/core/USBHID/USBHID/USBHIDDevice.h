//
//  USBHIDDevice.h
//  USBHID
//
//  Created by Christopher Stawarz on 4/3/13.
//  Copyright (c) 2013 The MWorks Project. All rights reserved.
//

#ifndef __USBHID__USBHIDDevice__
#define __USBHID__USBHIDDevice__

#include "USBHIDInputChannel.h"


BEGIN_NAMESPACE_MW


class USBHIDDevice : public IODevice, boost::noncopyable {
    
public:
    static const std::string USAGE_PAGE;
    static const std::string USAGE;
    static const std::string PREFERRED_LOCATION_ID;
    static const std::string LOG_ALL_INPUT_VALUES;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit USBHIDDevice(const ParameterValueMap &parameters);
    ~USBHIDDevice();
    
    void addChild(std::map<std::string, std::string> parameters,
                  ComponentRegistryPtr reg,
                  boost::shared_ptr<Component> child) override;
    
    bool initialize() override;
    bool startDeviceIO() override;
    bool stopDeviceIO() override;
    
private:
    using UsagePair = std::pair<long, long>;
    using HIDManagerPtr = cf::ObjectPtr<IOHIDManagerRef>;
    using HIDDevicePtr = cf::ObjectPtr<IOHIDDeviceRef>;
    using HIDElementPtr = cf::ObjectPtr<IOHIDElementRef>;
    
    static cf::DictionaryPtr createMatchingDictionary(CFStringRef usagePageKey,
                                                      long usagePage,
                                                      CFStringRef usageKey,
                                                      long usage);
    static void inputValueCallback(void *context, IOReturn result, void *sender, IOHIDValueRef value);
    
    bool prepareInputChannels();
    void runLoop();
    void handleInputValue(IOHIDValueRef value);
    
    const long usagePage;
    const long usage;
    const std::uint32_t preferredLocationID;
    const bool logAllInputValues;
    
    const HIDManagerPtr hidManager;
    
    std::map<UsagePair, boost::shared_ptr<USBHIDInputChannel>> inputChannels;
    
    HIDDevicePtr hidDevice;
    std::map<UsagePair, HIDElementPtr> hidElements;
    
    std::atomic_bool running;
    static_assert(decltype(running)::is_always_lock_free);
    std::thread runLoopThread;
    
};


END_NAMESPACE_MW


#endif // !defined(__USBHID__USBHIDDevice__)

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


BEGIN_NAMESPACE(iohid)


using DevicePtr = cf::ObjectPtr<IOHIDDeviceRef>;
using ElementPtr = cf::ObjectPtr<IOHIDElementRef>;
using ManagerPtr = cf::ObjectPtr<IOHIDManagerRef>;


END_NAMESPACE(iohid)


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
                  boost::shared_ptr<Component> child) MW_OVERRIDE;
    
    bool initialize() MW_OVERRIDE;
    bool startDeviceIO() MW_OVERRIDE;
    bool stopDeviceIO() MW_OVERRIDE;
    
private:
    static cf::DictionaryPtr createMatchingDictionary(CFStringRef usagePageKey,
                                                      long usagePage,
                                                      CFStringRef usageKey,
                                                      long usage);
    static void inputValueCallback(void *context, IOReturn result, void *sender, IOHIDValueRef value);
    
    bool prepareInputChannels();
    bool isRunning() const { return (runLoopThread.get_id() != boost::thread::id()); }
    void runLoop();
    void handleInputValue(IOHIDValueRef value);
    
    const long usagePage;
    const long usage;
    const std::uint32_t preferredLocationID;
    const bool logAllInputValues;
    
    typedef std::pair<long, long> UsagePair;
    typedef std::map< UsagePair, boost::shared_ptr<USBHIDInputChannel> > InputChannelMap;
    InputChannelMap inputChannels;
    
    const iohid::ManagerPtr hidManager;
    iohid::DevicePtr hidDevice;
    
    typedef std::map< UsagePair, iohid::ElementPtr > HIDElementMap;
    HIDElementMap hidElements;
    
    boost::thread runLoopThread;
    
};


END_NAMESPACE_MW


#endif // !defined(__USBHID__USBHIDDevice__)



























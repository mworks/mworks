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
    
    static cf::DictionaryPtr createMatchingDictionary(CFStringRef usagePageKey,
                                                      long usagePage,
                                                      CFStringRef usageKey,
                                                      long usage,
                                                      CFStringRef locationIDKey = nullptr,
                                                      CFNumberRef locationIDValue = nullptr);
    static bool getLocationID(IOHIDDeviceRef device, cf::NumberPtr &locationIDValue, std::uint32_t &locationID);
    
    static void deviceMatchingCallback(void *context, IOReturn result, void *sender, IOHIDDeviceRef device);
    static void deviceRemovalCallback(void *context, IOReturn result, void *sender, IOHIDDeviceRef device);
    static void inputValueCallback(void *context, IOReturn result, void *sender, IOHIDValueRef value);
    
    bool findMatchingDevice(HIDDevicePtr &hidDevice, cf::NumberPtr &locationIDValue);
    bool findMatchingElements(IOHIDDeviceRef device);
    void run();
    void handleDeviceMatching(IOHIDDeviceRef device);
    void handleDeviceRemoval(IOHIDDeviceRef device);
    void handleInputValue(IOHIDValueRef value);
    void postCurrentElementValues();
    void postElementValue(long elementUsagePage, long elementUsage, CFIndex elementIntegerValue, MWTime valueTime);
    
    const long usagePage;
    const long usage;
    const std::uint32_t preferredLocationID;
    const bool logAllInputValues;
    
    const boost::shared_ptr<Clock> clock;
    const HIDManagerPtr hidManager;
    
    std::map<UsagePair, boost::shared_ptr<USBHIDInputChannel>> inputChannels;
    
    using lock_guard = std::lock_guard<std::mutex>;
    lock_guard::mutex_type currentElementIntegerValuesMutex;
    std::map<UsagePair, CFIndex> currentElementIntegerValues;
    
    using atomic_bool = std::atomic_bool;
    static_assert(atomic_bool::is_always_lock_free);
    atomic_bool running;
    atomic_bool active;
    std::thread runThread;
    
    bool deviceWasRemoved;
    
};


END_NAMESPACE_MW


#endif // !defined(__USBHID__USBHIDDevice__)

//
//  USBHIDDevice.cpp
//  USBHID
//
//  Created by Christopher Stawarz on 4/3/13.
//  Copyright (c) 2013 The MWorks Project. All rights reserved.
//

#include "USBHIDDevice.h"


BEGIN_NAMESPACE_MW


const std::string USBHIDDevice::USAGE_PAGE("usage_page");
const std::string USBHIDDevice::USAGE("usage");
const std::string USBHIDDevice::PREFERRED_LOCATION_ID("preferred_location_id");
const std::string USBHIDDevice::LOG_ALL_INPUT_VALUES("log_all_input_values");


void USBHIDDevice::describeComponent(ComponentInfo &info) {
    IODevice::describeComponent(info);
    
    info.setSignature("iodevice/usbhid_generic");
    
    info.addParameter(USAGE_PAGE);
    info.addParameter(USAGE);
    info.addParameter(PREFERRED_LOCATION_ID, "0");
    info.addParameter(LOG_ALL_INPUT_VALUES, "NO");
}


USBHIDDevice::USBHIDDevice(const ParameterValueMap &parameters) :
    IODevice(parameters),
    usagePage(parameters[USAGE_PAGE]),
    usage(parameters[USAGE]),
    preferredLocationID(long(parameters[PREFERRED_LOCATION_ID])),
    logAllInputValues(parameters[LOG_ALL_INPUT_VALUES]),
    hidManager(iohid::ManagerPtr::created(IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDManagerOptionNone)))
{
    if (usagePage <= kHIDPage_Undefined) {
        throw SimpleException(M_IODEVICE_MESSAGE_DOMAIN, "Invalid HID usage page");
    }
    if (usage <= kHIDUsage_Undefined) {
        throw SimpleException(M_IODEVICE_MESSAGE_DOMAIN, "Invalid HID usage");
    }
}


USBHIDDevice::~USBHIDDevice() {
    (void)IOHIDManagerClose(hidManager.get(), kIOHIDOptionsTypeNone);
}


void USBHIDDevice::addChild(std::map<std::string, std::string> parameters,
                            ComponentRegistryPtr reg,
                            boost::shared_ptr<Component> child)
{
    boost::shared_ptr<USBHIDInputChannel> newInputChannel = boost::dynamic_pointer_cast<USBHIDInputChannel>(child);
    if (newInputChannel) {
        boost::shared_ptr<USBHIDInputChannel> &channel = inputChannels[std::make_pair(newInputChannel->getUsagePage(),
                                                                                      newInputChannel->getUsage())];
        if (channel) {
            throw SimpleException(M_IODEVICE_MESSAGE_DOMAIN,
                                  "Cannot create more than one USBHID channel for a given usage page and usage");
        }
        channel = newInputChannel;
        return;
    }
    
    throw SimpleException(M_IODEVICE_MESSAGE_DOMAIN, "Invalid channel type for USBHID device");
}


bool USBHIDDevice::initialize() {
    if (inputChannels.empty() && !logAllInputValues) {
        throw SimpleException(M_IODEVICE_MESSAGE_DOMAIN,
                              "USBHID device must have at least one channel or have value logging enabled");
    }
    
    cf::DictionaryPtr deviceMatchingDictionary = createMatchingDictionary(CFSTR(kIOHIDDeviceUsagePageKey),
                                                                          usagePage,
                                                                          CFSTR(kIOHIDDeviceUsageKey),
                                                                          usage);
    IOHIDManagerSetDeviceMatching(hidManager.get(), deviceMatchingDictionary.get());
    
    IOReturn status = IOHIDManagerOpen(hidManager.get(), kIOHIDOptionsTypeNone);
    if (kIOReturnSuccess != status) {
        merror(M_IODEVICE_MESSAGE_DOMAIN, "Unable to open HID manager (status = %d)", status);
        return false;
    }
    
    cf::SetPtr matchingDevices = cf::SetPtr::owned(IOHIDManagerCopyDevices(hidManager.get()));
    const CFIndex numMatchingDevices = (matchingDevices ? CFSetGetCount(matchingDevices.get()) : 0);
    if (!matchingDevices || (numMatchingDevices < 1)) {
        merror(M_IODEVICE_MESSAGE_DOMAIN, "No matching HID devices found");
        return false;
    }
    
    std::vector<IOHIDDeviceRef> devices(numMatchingDevices);
    CFSetGetValues(matchingDevices.get(), (const void **)(devices.data()));
    
    if (numMatchingDevices == 1) {
        
        hidDevice = iohid::DevicePtr::borrowed(devices[0]);
        
        if (preferredLocationID) {
            CFNumberRef locationID = static_cast<CFNumberRef>(IOHIDDeviceGetProperty(hidDevice.get(), CFSTR(kIOHIDLocationIDKey)));
            if (locationID) {
                std::uint32_t value;
                CFNumberGetValue(locationID, kCFNumberSInt32Type, &value);
                if (preferredLocationID != value) {
                    mwarning(M_IODEVICE_MESSAGE_DOMAIN,
                             "Location ID for HID device \"%s\" (%lu) does not match preferred location ID (%lu)",
                             getTag().c_str(),
                             static_cast<unsigned long>(value),
                             static_cast<unsigned long>(preferredLocationID));
                }
            }
        }
        
    } else {
        
        std::ostringstream oss;
        
        oss << "Found multiple matching HID devices for \"" << getTag() << "\":\n";
        
        for (CFIndex deviceNum = 0; deviceNum < numMatchingDevices; deviceNum++) {
            oss << "\nDevice #" << std::dec << deviceNum + 1 << std::endl;
            
            IOHIDDeviceRef device = devices[deviceNum];
            std::vector<char> stringBuffer(1024);
            
            CFStringRef product = static_cast<CFStringRef>(IOHIDDeviceGetProperty(device, CFSTR(kIOHIDProductKey)));
            if (product) {
                CFStringGetCString(product, stringBuffer.data(), stringBuffer.size(), kCFStringEncodingUTF8);
                oss << "\tProduct:\t\t" << stringBuffer.data() << std::endl;
            }
            
            CFStringRef manufacturer = static_cast<CFStringRef>(IOHIDDeviceGetProperty(device, CFSTR(kIOHIDManufacturerKey)));
            if (manufacturer) {
                CFStringGetCString(manufacturer, stringBuffer.data(), stringBuffer.size(), kCFStringEncodingUTF8);
                oss << "\tManufacturer:\t" << stringBuffer.data() << std::endl;
            }
            
            CFNumberRef locationID = static_cast<CFNumberRef>(IOHIDDeviceGetProperty(device, CFSTR(kIOHIDLocationIDKey)));
            if (locationID) {
                std::uint32_t value;
                CFNumberGetValue(locationID, kCFNumberSInt32Type, &value);
                
                if (preferredLocationID == value) {
                    hidDevice = iohid::DevicePtr::borrowed(device);
                    break;
                }
                
                oss << "\tLocation ID:\t" << std::dec << value << "\t(0x" << std::hex << value << ")" << std::endl;
            }
        }
        
        if (!hidDevice) {
            oss << "\nPlease set the \"" << PREFERRED_LOCATION_ID
                << "\" attribute to the Location ID of the desired device.\n";
            merror(M_IODEVICE_MESSAGE_DOMAIN, "%s", oss.str().c_str());
            return false;
        }
        
    }
    
    if (!prepareInputChannels()) {
        return false;
    }
    
    IOHIDDeviceRegisterInputValueCallback(hidDevice.get(), &inputValueCallback, this);
    
    return true;
}


bool USBHIDDevice::startDeviceIO() {
    if (!isRunning()) {
        BOOST_FOREACH(const HIDElementMap::value_type &value, hidElements) {
            IOHIDValueRef elementValue;
            IOReturn status;
            if (kIOReturnSuccess == (status = IOHIDDeviceGetValue(hidDevice.get(),
                                                                  value.second.get(),
                                                                  &elementValue)))
            {
                handleInputValue(elementValue);
            } else {
                merror(M_IODEVICE_MESSAGE_DOMAIN, "HID value get failed: %s", mach_error_string(status));
                return false;
            }
        }
        
        try {
            runLoopThread = boost::thread(boost::bind(&USBHIDDevice::runLoop,
                                                      component_shared_from_this<USBHIDDevice>()));
        } catch (const boost::thread_resource_error &e) {
            merror(M_IODEVICE_MESSAGE_DOMAIN, "Unable to start HID device: %s", e.what());
            return false;
        }
    }
    
    return true;
}


bool USBHIDDevice::stopDeviceIO() {
    if (isRunning()) {
        runLoopThread.interrupt();
        try {
            runLoopThread.join();
        } catch (const boost::system::system_error &e) {
            merror(M_IODEVICE_MESSAGE_DOMAIN, "Unable to stop HID device: %s", e.what());
            return false;
        }
    }
    
    return true;
}


cf::DictionaryPtr USBHIDDevice::createMatchingDictionary(CFStringRef usagePageKey,
                                                         long usagePageValue,
                                                         CFStringRef usageKey,
                                                         long usageValue)
{
    cf::NumberPtr usagePage = cf::NumberPtr::created(CFNumberCreate(kCFAllocatorDefault,
                                                                    kCFNumberLongType,
                                                                    &usagePageValue));
    
    cf::NumberPtr usage = cf::NumberPtr::created(CFNumberCreate(kCFAllocatorDefault,
                                                                kCFNumberLongType,
                                                                &usageValue));
    
    const CFIndex numValues = 2;
    const void *keys[numValues] = {usagePageKey, usageKey};
    const void *values[numValues] = {usagePage.get(), usage.get()};
    
    return cf::DictionaryPtr::created(CFDictionaryCreate(kCFAllocatorDefault,
                                                         keys,
                                                         values,
                                                         numValues,
                                                         &kCFTypeDictionaryKeyCallBacks,
                                                         &kCFTypeDictionaryValueCallBacks));
}


void USBHIDDevice::inputValueCallback(void *context, IOReturn result, void *sender, IOHIDValueRef value) {
    static_cast<USBHIDDevice *>(context)->handleInputValue(value);
}


bool USBHIDDevice::prepareInputChannels() {
    std::vector<cf::DictionaryPtr> matchingDicts;
    std::vector<const void *> matchingArrayItems;
    
    BOOST_FOREACH(const InputChannelMap::value_type &value, inputChannels) {
        const UsagePair &usagePair = value.first;
        
        cf::DictionaryPtr dict = createMatchingDictionary(CFSTR(kIOHIDElementUsagePageKey),
                                                          usagePair.first,
                                                          CFSTR(kIOHIDElementUsageKey),
                                                          usagePair.second);
        matchingDicts.push_back(dict);
        matchingArrayItems.push_back(dict.get());
        
        cf::ArrayPtr matchingElements = cf::ArrayPtr::owned(IOHIDDeviceCopyMatchingElements(hidDevice.get(),
                                                                                            dict.get(),
                                                                                            kIOHIDOptionsTypeNone));
        bool foundMatch = false;
        
        if (matchingElements) {
            const CFIndex matchingElementCount = CFArrayGetCount(matchingElements.get());
            
            for (CFIndex index = 0; index < matchingElementCount; index++) {
                IOHIDElementRef element = (IOHIDElementRef)CFArrayGetValueAtIndex(matchingElements.get(), index);
                IOHIDElementType elementType = IOHIDElementGetType(element);
                
                if ((elementType != kIOHIDElementTypeFeature) && (elementType != kIOHIDElementTypeCollection)) {
                    hidElements[usagePair] = iohid::ElementPtr::borrowed(element);
                    foundMatch = true;
                    break;
                }
            }
        }
        
        if (!foundMatch) {
            merror(M_IODEVICE_MESSAGE_DOMAIN,
                   "No matching HID elements for usage page %ld, usage %ld",
                   usagePair.first,
                   usagePair.second);
            return false;
        }
    }
    
    if (!logAllInputValues) {
        cf::ArrayPtr inputValueMatchingArray = cf::ArrayPtr::created(CFArrayCreate(kCFAllocatorDefault,
                                                                                   &(matchingArrayItems.front()),
                                                                                   matchingArrayItems.size(),
                                                                                   &kCFTypeArrayCallBacks));
        
        IOHIDDeviceSetInputValueMatchingMultiple(hidDevice.get(), inputValueMatchingArray.get());
    }
    
    return true;
}


void USBHIDDevice::runLoop() {
    IOHIDManagerScheduleWithRunLoop(hidManager.get(), CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
    
    BOOST_SCOPE_EXIT(&hidManager) {
        IOHIDManagerUnscheduleFromRunLoop(hidManager.get(), CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
    } BOOST_SCOPE_EXIT_END
    
    while (true) {
        // Run the CFRunLoop for 500ms
        (void)CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.5, false);
        
        // Give another thread a chance to terminate this one
        boost::this_thread::interruption_point();
    }
}


void USBHIDDevice::handleInputValue(IOHIDValueRef value) {
    IOHIDElementRef element = IOHIDValueGetElement(value);
    const uint32_t elementUsagePage = IOHIDElementGetUsagePage(element);
    const uint32_t elementUsage = IOHIDElementGetUsage(element);
    const CFIndex integerValue = IOHIDValueGetIntegerValue(value);
    
    const boost::shared_ptr<USBHIDInputChannel> &channel = inputChannels[std::make_pair(long(elementUsagePage),
                                                                                        long(elementUsage))];
    if (channel) {
        // Convert OS absolute time stamp to nanoseconds, subtract MWorks base time, and convert to microseconds
        MWTime valueTime = ((MWTime(AudioConvertHostTimeToNanos(IOHIDValueGetTimeStamp(value)))
                                    - Clock::instance()->getSystemBaseTimeNS())
                            / MWTime(1000));
        
        channel->postValue(integerValue, valueTime);
    }
    
    if (logAllInputValues) {
        mprintf("HID input on device \"%s\":\n"
                "\tUsage page:\t%u\t(0x%02X)\n"
                "\tUsage:\t\t%u\t(0x%02X)\n"
                "\tValue:\t\t%ld",
                getTag().c_str(),
                elementUsagePage, elementUsagePage,
                elementUsage, elementUsage,
                integerValue);
    }
}


END_NAMESPACE_MW



























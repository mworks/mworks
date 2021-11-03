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
    clock(Clock::instance()),
    hidManager(HIDManagerPtr::created(IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDManagerOptionNone))),
    running(false),
    active(false),
    deviceWasRemoved(false)
{
    if (usagePage <= kHIDPage_Undefined) {
        throw SimpleException(M_IODEVICE_MESSAGE_DOMAIN, "Invalid HID usage page");
    }
    if (usage <= kHIDUsage_Undefined) {
        throw SimpleException(M_IODEVICE_MESSAGE_DOMAIN, "Invalid HID usage");
    }
}


USBHIDDevice::~USBHIDDevice() {
    if (runThread.joinable()) {
        running = false;
        runThread.join();
    }
    (void)IOHIDManagerClose(hidManager.get(), kIOHIDOptionsTypeNone);
}


void USBHIDDevice::addChild(std::map<std::string, std::string> parameters,
                            ComponentRegistryPtr reg,
                            boost::shared_ptr<Component> child)
{
    auto newInputChannel = boost::dynamic_pointer_cast<USBHIDInputChannel>(child);
    if (newInputChannel) {
        auto &channel = inputChannels[std::make_pair(newInputChannel->getUsagePage(), newInputChannel->getUsage())];
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
    
    auto deviceMatchingDictionary = createMatchingDictionary(CFSTR(kIOHIDDeviceUsagePageKey),
                                                             usagePage,
                                                             CFSTR(kIOHIDDeviceUsageKey),
                                                             usage);
    IOHIDManagerSetDeviceMatching(hidManager.get(), deviceMatchingDictionary.get());
    
    auto status = IOHIDManagerOpen(hidManager.get(), kIOHIDOptionsTypeNone);
    if (kIOReturnSuccess != status) {
        merror(M_IODEVICE_MESSAGE_DOMAIN, "Unable to open HID manager (status = %d)", status);
        return false;
    }
    
    HIDDevicePtr hidDevice;
    cf::NumberPtr locationIDValue;
    if (!findMatchingDevice(hidDevice, locationIDValue)) {
        return false;
    }
    
    if (!locationIDValue) {
        merror(M_IODEVICE_MESSAGE_DOMAIN, "Cannot determine location ID for HID device \"%s\"", getTag().c_str());
        return false;
    }
    
    if (!findMatchingElements(hidDevice.get())) {
        return false;
    }
    
    // Update the matching dictionary to match only the selected device
    deviceMatchingDictionary = createMatchingDictionary(CFSTR(kIOHIDDeviceUsagePageKey),
                                                        usagePage,
                                                        CFSTR(kIOHIDDeviceUsageKey),
                                                        usage,
                                                        CFSTR(kIOHIDLocationIDKey),
                                                        locationIDValue.get());
    IOHIDManagerSetDeviceMatching(hidManager.get(), deviceMatchingDictionary.get());
    
    IOHIDManagerRegisterDeviceMatchingCallback(hidManager.get(), &deviceMatchingCallback, this);
    IOHIDManagerRegisterDeviceRemovalCallback(hidManager.get(), &deviceRemovalCallback, this);
    IOHIDManagerRegisterInputValueCallback(hidManager.get(), &inputValueCallback, this);
    
    running = true;
    runThread = std::thread([this]() { run(); });
    
    return true;
}


bool USBHIDDevice::startDeviceIO() {
    if (!active) {
        postCurrentElementValues();
        active = true;
    }
    
    return true;
}


bool USBHIDDevice::stopDeviceIO() {
    if (active) {
        active = false;
    }
    
    return true;
}


cf::DictionaryPtr USBHIDDevice::createMatchingDictionary(CFStringRef usagePageKey,
                                                         long usagePage,
                                                         CFStringRef usageKey,
                                                         long usage,
                                                         CFStringRef locationIDKey,
                                                         CFNumberRef locationIDValue)
{
    auto usagePageValue = cf::NumberPtr::created(CFNumberCreate(kCFAllocatorDefault, kCFNumberLongType, &usagePage));
    auto usageValue = cf::NumberPtr::created(CFNumberCreate(kCFAllocatorDefault, kCFNumberLongType, &usage));
    
    const void *keys[] = { usagePageKey, usageKey, locationIDKey };
    const void *values[] = { usagePageValue.get(), usageValue.get(), locationIDValue };
    
    return cf::DictionaryPtr::created(CFDictionaryCreate(kCFAllocatorDefault,
                                                         keys,
                                                         values,
                                                         ((locationIDKey && locationIDValue) ? 3 : 2),
                                                         &kCFTypeDictionaryKeyCallBacks,
                                                         &kCFTypeDictionaryValueCallBacks));
}


bool USBHIDDevice::getLocationID(IOHIDDeviceRef device, cf::NumberPtr &locationIDValue, std::uint32_t &locationID) {
    locationIDValue = cf::NumberPtr::borrowed(static_cast<CFNumberRef>(IOHIDDeviceGetProperty(device,
                                                                                              CFSTR(kIOHIDLocationIDKey))));
    if (!locationIDValue) {
        return false;
    }
    return CFNumberGetValue(locationIDValue.get(), kCFNumberSInt32Type, &locationID);
}


void USBHIDDevice::deviceMatchingCallback(void *context, IOReturn result, void *sender, IOHIDDeviceRef device) {
    static_cast<USBHIDDevice *>(context)->handleDeviceMatching(device);
}


void USBHIDDevice::deviceRemovalCallback(void *context, IOReturn result, void *sender, IOHIDDeviceRef device) {
    static_cast<USBHIDDevice *>(context)->handleDeviceRemoval(device);
}


void USBHIDDevice::inputValueCallback(void *context, IOReturn result, void *sender, IOHIDValueRef value) {
    static_cast<USBHIDDevice *>(context)->handleInputValue(value);
}


bool USBHIDDevice::findMatchingDevice(HIDDevicePtr &hidDevice, cf::NumberPtr &locationIDValue) {
    auto matchingDevices = cf::SetPtr::owned(IOHIDManagerCopyDevices(hidManager.get()));
    const CFIndex numMatchingDevices = (matchingDevices ? CFSetGetCount(matchingDevices.get()) : 0);
    if (!matchingDevices || (numMatchingDevices < 1)) {
        merror(M_IODEVICE_MESSAGE_DOMAIN, "No matching HID devices found");
        return false;
    }
    
    std::vector<IOHIDDeviceRef> devices(numMatchingDevices);
    CFSetGetValues(matchingDevices.get(), (const void **)(devices.data()));
    
    if (numMatchingDevices == 1) {
        
        hidDevice = HIDDevicePtr::borrowed(devices[0]);
        
        std::uint32_t locationID;
        if (getLocationID(hidDevice.get(), locationIDValue, locationID) &&
            preferredLocationID &&
            preferredLocationID != locationID)
        {
            mwarning(M_IODEVICE_MESSAGE_DOMAIN,
                     "Location ID for HID device \"%s\" (%lu) does not match preferred location ID (%lu)",
                     getTag().c_str(),
                     static_cast<unsigned long>(locationID),
                     static_cast<unsigned long>(preferredLocationID));
        }
        
    } else {
        
        std::ostringstream oss;
        
        oss << "Found multiple matching HID devices for \"" << getTag() << "\":\n";
        
        for (CFIndex deviceNum = 0; deviceNum < numMatchingDevices; deviceNum++) {
            oss << "\nDevice #" << std::dec << deviceNum + 1 << std::endl;
            
            auto &device = devices[deviceNum];
            std::array<char, 1024> stringBuffer;
            
            auto product = static_cast<CFStringRef>(IOHIDDeviceGetProperty(device, CFSTR(kIOHIDProductKey)));
            if (product) {
                CFStringGetCString(product, stringBuffer.data(), stringBuffer.size(), kCFStringEncodingUTF8);
                oss << "\tProduct:\t\t" << stringBuffer.data() << std::endl;
            }
            
            auto manufacturer = static_cast<CFStringRef>(IOHIDDeviceGetProperty(device, CFSTR(kIOHIDManufacturerKey)));
            if (manufacturer) {
                CFStringGetCString(manufacturer, stringBuffer.data(), stringBuffer.size(), kCFStringEncodingUTF8);
                oss << "\tManufacturer:\t" << stringBuffer.data() << std::endl;
            }
            
            std::uint32_t locationID;
            if (getLocationID(device, locationIDValue, locationID)) {
                if (preferredLocationID == locationID) {
                    hidDevice = HIDDevicePtr::borrowed(device);
                    break;
                }
                
                oss << "\tLocation ID:\t" << std::dec << locationID << "\t(0x" << std::hex << locationID << ")" << std::endl;
            }
        }
        
        if (!hidDevice) {
            oss << "\nPlease set the \"" << PREFERRED_LOCATION_ID
                << "\" attribute to the Location ID of the desired device.\n";
            merror(M_IODEVICE_MESSAGE_DOMAIN, "%s", oss.str().c_str());
            return false;
        }
        
    }
    
    return true;
}


bool USBHIDDevice::findMatchingElements(IOHIDDeviceRef device) {
    lock_guard lock(currentElementIntegerValuesMutex);
    
    currentElementIntegerValues.clear();
    
    std::vector<cf::DictionaryPtr> matchingDicts;
    std::vector<const void *> matchingArrayItems;
    
    for (const auto &value : inputChannels) {
        auto &usagePair = value.first;
        
        auto dict = createMatchingDictionary(CFSTR(kIOHIDElementUsagePageKey),
                                             usagePair.first,
                                             CFSTR(kIOHIDElementUsageKey),
                                             usagePair.second);
        matchingDicts.push_back(dict);
        matchingArrayItems.push_back(dict.get());
        
        auto matchingElements = cf::ArrayPtr::owned(IOHIDDeviceCopyMatchingElements(device,
                                                                                    dict.get(),
                                                                                    kIOHIDOptionsTypeNone));
        bool foundMatch = false;
        
        if (matchingElements) {
            const auto matchingElementCount = CFArrayGetCount(matchingElements.get());
            
            for (CFIndex index = 0; index < matchingElementCount; index++) {
                auto element = (IOHIDElementRef)CFArrayGetValueAtIndex(matchingElements.get(), index);
                auto elementType = IOHIDElementGetType(element);
                
                if ((elementType != kIOHIDElementTypeFeature) && (elementType != kIOHIDElementTypeCollection)) {
                    foundMatch = true;
                    
                    // Retrieve and store the current value of the element
                    IOHIDValueRef elementValue;
                    IOReturn status;
                    if (kIOReturnSuccess == (status = IOHIDDeviceGetValue(device, element, &elementValue))) {
                        currentElementIntegerValues[usagePair] = IOHIDValueGetIntegerValue(elementValue);
                    } else {
                        merror(M_IODEVICE_MESSAGE_DOMAIN, "HID value get failed: %s", mach_error_string(status));
                    }
                    
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
        auto inputValueMatchingArray = cf::ArrayPtr::created(CFArrayCreate(kCFAllocatorDefault,
                                                                           &(matchingArrayItems.front()),
                                                                           matchingArrayItems.size(),
                                                                           &kCFTypeArrayCallBacks));
        
        IOHIDManagerSetInputValueMatchingMultiple(hidManager.get(), inputValueMatchingArray.get());
    }
    
    return true;
}


void USBHIDDevice::run() {
    IOHIDManagerScheduleWithRunLoop(hidManager.get(), CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
    
    BOOST_SCOPE_EXIT(&hidManager) {
        IOHIDManagerUnscheduleFromRunLoop(hidManager.get(), CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
    } BOOST_SCOPE_EXIT_END
    
    while (running) {
        // Run the CFRunLoop for 500ms
        (void)CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.5, false);
    }
}


void USBHIDDevice::handleDeviceMatching(IOHIDDeviceRef device) {
    if (!deviceWasRemoved) {
        return;
    }
    
    mprintf(M_IODEVICE_MESSAGE_DOMAIN, "Attempting to reconnect to HID device \"%s\"...", getTag().c_str());

    if (!findMatchingElements(device)) {
        return;
    }
    
    mprintf(M_IODEVICE_MESSAGE_DOMAIN, "HID device \"%s\" reconnected successfully", getTag().c_str());
    deviceWasRemoved = false;
    
    if (active) {
        postCurrentElementValues();
    }
}


void USBHIDDevice::handleDeviceRemoval(IOHIDDeviceRef device) {
    if (deviceWasRemoved) {
        return;
    }
    
    merror(M_IODEVICE_MESSAGE_DOMAIN, "HID device \"%s\" was removed", getTag().c_str());
    deviceWasRemoved = true;
}


void USBHIDDevice::handleInputValue(IOHIDValueRef value) {
    auto element = IOHIDValueGetElement(value);
    const auto elementUsagePage = IOHIDElementGetUsagePage(element);
    const auto elementUsage = IOHIDElementGetUsage(element);
    const auto elementIntegerValue = IOHIDValueGetIntegerValue(value);
    
    {
        lock_guard lock(currentElementIntegerValuesMutex);
        currentElementIntegerValues[std::make_pair(elementUsagePage, elementUsage)] = elementIntegerValue;
    }
    
    if (active) {
        // Convert OS absolute time stamp to nanoseconds, subtract MWorks base time, and convert to microseconds
        const MWTime valueTime = ((MWTime(AudioConvertHostTimeToNanos(IOHIDValueGetTimeStamp(value)))
                                   - clock->getSystemBaseTimeNS())
                                  / MWTime(1000));
        
        postElementValue(elementUsagePage, elementUsage, elementIntegerValue, valueTime);
        
        if (logAllInputValues) {
            mprintf("HID input on device \"%s\":\n"
                    "\tUsage page:\t%u\t(0x%02X)\n"
                    "\tUsage:\t\t%u\t(0x%02X)\n"
                    "\tValue:\t\t%ld",
                    getTag().c_str(),
                    elementUsagePage, elementUsagePage,
                    elementUsage, elementUsage,
                    elementIntegerValue);
        }
    }
}


void USBHIDDevice::postCurrentElementValues() {
    lock_guard lock(currentElementIntegerValuesMutex);
    
    const auto valueTime = clock->getCurrentTimeUS();
    
    for (const auto &item : currentElementIntegerValues) {
        auto &usagePair = item.first;
        postElementValue(usagePair.first, usagePair.second, item.second, valueTime);
    }
}


void USBHIDDevice::postElementValue(long elementUsagePage,
                                    long elementUsage,
                                    CFIndex elementIntegerValue,
                                    MWTime valueTime)
{
    const auto &channel = inputChannels[std::make_pair(elementUsagePage, elementUsage)];
    if (channel) {
        channel->postValue(elementIntegerValue, valueTime);
    }
}


END_NAMESPACE_MW

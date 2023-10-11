//
//  SerialPort.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 6/30/16.
//  Copyright © 2016 The MWorks Project. All rights reserved.
//

//
// NOTE: The methods used here to discover, connect to, and communicate with serial ports come from Apple's
// "Performing Serial I/O" sample code, which is available at
// https://developer.apple.com/library/mac/samplecode/SerialPortSample/Introduction/Intro.html
//

#include "SerialPort.hpp"

#include <fcntl.h>
#include <sys/ioctl.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/serial/IOSerialKeys.h>
#include <IOKit/serial/ioss.h>
#include <IOKit/IOBSD.h>

#include <boost/filesystem.hpp>
#include <boost/scope_exit.hpp>

#include "CFObjectPtr.h"
#include "Utilities.h"


BEGIN_NAMESPACE_MW


SerialPort::SerialPort(bool blocking) :
    blocking(blocking),
    fd(-1)
{ }


SerialPort::~SerialPort() {
    disconnect();
}


bool SerialPort::connect(std::string path, speed_t baudRate) {
    if (-1 != fd) {
        merror(M_IODEVICE_MESSAGE_DOMAIN, "Serial port is already connected");
        return false;
    }
    
    if (!validatePath(path)) {
        return false;
    }
    
    // Open the serial port read/write, with no controlling terminal, and don't wait for a connection.
    // The O_NONBLOCK flag also causes subsequent I/O on the device to be non-blocking.
    if (-1 == (fd = ::open(path.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK))) {
        sysError("Cannot open serial port");
        return false;
    }
    
    bool shouldClose = true;
    BOOST_SCOPE_EXIT(&shouldClose, &fd) {
        if (shouldClose) {
            (void)::close(fd);
            fd = -1;
        }
    } BOOST_SCOPE_EXIT_END
    
    // open() follows POSIX semantics: multiple open() calls to the same file will succeed
    // unless the TIOCEXCL ioctl is issued.  This will prevent additional opens except by root-owned
    // processes.
    if (-1 == ioctl(fd, TIOCEXCL)) {
        sysError("Cannot obtain exclusive use of serial port");
        return false;
    }
    
    if (blocking) {
        // Now that the device is open, clear the O_NONBLOCK flag so subsequent I/O will block
        if (-1 == fcntl(fd, F_SETFL, 0)) {
            sysError("Cannot restore blocking I/O on serial port");
            return false;
        }
    }
    
    // Get the current options and save them, so we can restore the default settings later
    if (-1 == tcgetattr(fd, &origAttrs)) {
        sysError("Cannot obtain current serial port attributes");
        return false;
    }
    
    struct termios attrs = origAttrs;
    cfmakeraw(&attrs);            // Set raw input (non-canonical) mode
    if (blocking) {
        attrs.c_cc[VMIN] = 0;     // Reads block until a single byte has been received
        attrs.c_cc[VTIME] = 5;    //   or a 500ms timeout expires
    }
    cfsetspeed(&attrs, baudRate); // Set baud rate
    attrs.c_cflag |= CS8;         // Use 8-bit words
    attrs.c_cflag &= ~PARENB;     // No parity
    attrs.c_cflag &= ~CSTOPB;     // 1 stop bit
    attrs.c_cflag |= CLOCAL;      // Ignore modem status lines
    
    // Cause the new options to take effect immediately
    if (-1 == tcsetattr(fd, TCSANOW, &attrs)) {
        sysError("Cannot set serial port attributes");
        return false;
    }
    
    shouldClose = false;
    
    return true;
}


void SerialPort::disconnect() {
    if (-1 == fd) {
        return;
    }
    
    // Block until all written output has been sent to the device
    if (-1 == tcdrain(fd)) {
        sysError("Serial port drain failed");
    }
    
    // Restore original options
    if (-1 == tcsetattr(fd, TCSANOW, &origAttrs)) {
        sysError("Cannot restore previous serial port attributes");
    }
    
    if (-1 == ::close(fd)) {
        sysError("Cannot close serial port");
    }
    
    fd = -1;
}


ssize_t SerialPort::read(void *data, std::size_t size) {
    auto result = ::read(fd, data, size);
    if (-1 == result) {
        if (!blocking && (EAGAIN == errno)) {
            // Non-blocking read with no data available
            return 0;
        }
        sysError("Read from serial port failed");
    }
    return result;
}


ssize_t SerialPort::write(const void *data, std::size_t size) {
    auto result = ::write(fd, data, size);
    if (-1 == result) {
        sysError("Write to serial port failed");
    }
    return result;
}


bool SerialPort::validatePath(std::string &path) {
    auto classesToMatch = cf::ObjectPtr<CFMutableDictionaryRef>::owned(IOServiceMatching(kIOSerialBSDServiceValue));
    if (classesToMatch) {
        CFDictionarySetValue(classesToMatch.get(), CFSTR(kIOSerialBSDTypeKey), CFSTR(kIOSerialBSDAllTypes));
    }
    
    io_iterator_t matchingServices = IO_OBJECT_NULL;
    if (KERN_SUCCESS != IOServiceGetMatchingServices(kIOMainPortDefault,
                                                     classesToMatch.release(),  // Reference is consumed by callee
                                                     &matchingServices))
    {
        merror(M_IODEVICE_MESSAGE_DOMAIN, "Failed to detect available serial devices");
        return false;
    }
    BOOST_SCOPE_EXIT(matchingServices) {
        IOObjectRelease(matchingServices);
    } BOOST_SCOPE_EXIT_END
    
    std::vector<std::string> devicePaths;
    io_object_t service = IO_OBJECT_NULL;
    while ((service = IOIteratorNext(matchingServices))) {
        BOOST_SCOPE_EXIT(service) {
            IOObjectRelease(service);
        } BOOST_SCOPE_EXIT_END
        
        auto pathAsCFString = cf::StringPtr::owned(static_cast<CFStringRef>(IORegistryEntryCreateCFProperty(service,
                                                                                                            CFSTR(kIOCalloutDeviceKey),
                                                                                                            kCFAllocatorDefault,
                                                                                                            0)));
        if (pathAsCFString &&
            // Ignore known paths that don't correspond to connected, physical devices
            kCFNotFound == CFStringFind(pathAsCFString.get(), CFSTR("BLTH"), 0).location &&
            kCFNotFound == CFStringFind(pathAsCFString.get(), CFSTR("Bluetooth-"), 0).location &&
            kCFNotFound == CFStringFind(pathAsCFString.get(), CFSTR("wlan-debug"), 0).location)
        {
            std::vector<char> buffer(CFStringGetMaximumSizeForEncoding(CFStringGetLength(pathAsCFString.get()),
                                                                       kCFStringEncodingUTF8) + 1);
            if (CFStringGetCString(pathAsCFString.get(), buffer.data(), buffer.size(), kCFStringEncodingUTF8)) {
                devicePaths.emplace_back(buffer.data());
            }
        }
    }
    
    if (!path.empty()) {
        // Try to canonicalize the path (make absolute, resolve symbolic links, etc.)
        boost::system::error_code ec;
        auto canonPath = boost::filesystem::canonical(boost::filesystem::path(path), ec);
        if (!ec) {
            path = canonPath.string();
        }
        
        if (std::find(devicePaths.begin(), devicePaths.end(), path) != devicePaths.end()) {
            // Requested path exists and is a serial device
            return true;
        }
        merror(M_IODEVICE_MESSAGE_DOMAIN, "Requested path (%s) does not exist or is not a serial device", path.c_str());
        return false;
    }
    
    if (devicePaths.empty()) {
        merror(M_IODEVICE_MESSAGE_DOMAIN, "No serial devices found");
        return false;
    }
    
    if (devicePaths.size() > 1) {
        std::ostringstream oss;
        oss << "Multiple candidate serial devices found:\n";
        for (const auto &p : devicePaths) {
            oss << "\n\t" << p;
        }
        oss << "\n\nPlease provide the explicit path for the desired device.\n";
        merror(M_IODEVICE_MESSAGE_DOMAIN, "%s", oss.str().c_str());
        return false;
    }
    
    // Only one device found.  Assume it's the one we want.
    path = devicePaths.front();
    return true;
}


inline void SerialPort::sysError(const std::string &msg) {
    merror(M_IODEVICE_MESSAGE_DOMAIN, "%s: %s", msg.c_str(), strerror(errno));
}


END_NAMESPACE_MW

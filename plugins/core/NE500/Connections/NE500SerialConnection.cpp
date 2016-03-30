//
//  NE500SerialConnection.cpp
//  NE500Plugin
//
//  Created by Christopher Stawarz on 3/29/16.
//
//

#include "NE500SerialConnection.hpp"

#include <sys/ioctl.h>


BEGIN_NAMESPACE_MW


bool NE500SerialConnection::connect() {
    if (connected) {
        return true;
    }
    
    // Open the serial port read/write, with no controlling terminal, and don't wait for a connection.
    // The O_NONBLOCK flag also causes subsequent I/O on the device to be non-blocking.
    if (-1 == (fd = ::open(path.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK))) {
        merror(M_IODEVICE_MESSAGE_DOMAIN, "Cannot open serial port (%s): %s", path.c_str(), strerror(errno));
        return false;
    }
    
    // open() follows POSIX semantics: multiple open() calls to the same file will succeed
    // unless the TIOCEXCL ioctl is issued.  This will prevent additional opens except by root-owned
    // processes.
    if (-1 == ioctl(fd, TIOCEXCL)) {
        merror(M_IODEVICE_MESSAGE_DOMAIN, "Cannot obtain exclusive use of serial port: %s", strerror(errno));
        close(fd);
        return false;
    }
    
    // Get the current options and save them, so we can restore the default settings later
    if (-1 == tcgetattr(fd, &origAttrs)) {
        merror(M_IODEVICE_MESSAGE_DOMAIN, "Cannot obtain current serial port attributes: %s", strerror(errno));
        close(fd);
        return false;
    }
    
    struct termios attrs = origAttrs;
    cfmakeraw(&attrs);           // Set raw input (non-canonical) mode
    cfsetspeed(&attrs, B19200);  // Set speed to 19200 baud
    attrs.c_cflag |= CS8;        // Use 8-bit words
    attrs.c_cflag &= ~PARENB;    // No parity
    attrs.c_cflag &= ~CSTOPB;    // 1 stop bit
    
    // Cause the new options to take effect immediately
    if (-1 == tcsetattr(fd, TCSANOW, &attrs)) {
        merror(M_IODEVICE_MESSAGE_DOMAIN, "Cannot set serial port attributes: %s", strerror(errno));
        close(fd);
        return false;
    }
    
    connected = true;
    return true;
}


bool NE500SerialConnection::disconnect() {
    if (!connected) {
        return true;
    }
    
    // Block until all written output has been sent to the device
    if (-1 == tcdrain(fd)) {
        merror(M_IODEVICE_MESSAGE_DOMAIN, "Serial port drain failed: %s", strerror(errno));
    }
    
    // Restore original options
    if (-1 == tcsetattr(fd, TCSANOW, &origAttrs)) {
        merror(M_IODEVICE_MESSAGE_DOMAIN, "Cannot restore previous serial port attributes: %s", strerror(errno));
    }
    
    if (-1 == ::close(fd)) {
        merror(M_IODEVICE_MESSAGE_DOMAIN, "Cannot close serial port: %s", strerror(errno));
    }
    
    connected = false;
    return true;
}


bool NE500SerialConnection::send(const std::string &command) {
    if (!connected) {
        return false;
    }
    
    if (-1 == ::write(fd, command.data(), command.size())) {
        merror(M_IODEVICE_MESSAGE_DOMAIN, "Cannot send data to NE500 device: %s", strerror(errno));
        return false;
    }
    
    return true;
}


bool NE500SerialConnection::receive(std::string &response) {
    if (!connected) {
        return false;
    }
    
    std::array<char, 512> buffer;
    auto rc = ::read(fd, buffer.data(), buffer.size());
    
    if (rc >= 0) {
        
        response.append(buffer.data(), rc);
        
    } else if (errno != EWOULDBLOCK) {
        
        merror(M_IODEVICE_MESSAGE_DOMAIN, "Cannot receive data from NE500 device: %s", strerror(errno));
        return false;
        
    }
    
    return true;
}


END_NAMESPACE_MW




























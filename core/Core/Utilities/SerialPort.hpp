//
//  SerialPort.hpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 6/30/16.
//  Copyright © 2016 The MWorks Project. All rights reserved.
//

#ifndef MWorksCore_SerialPort_hpp
#define MWorksCore_SerialPort_hpp

#include <string>
#include <termios.h>

#include <boost/noncopyable.hpp>

#include <MWorksCore/MWorksMacros.h>


BEGIN_NAMESPACE_MW


class SerialPort : boost::noncopyable {
    
public:
    explicit SerialPort(bool blocking = true);
    ~SerialPort();
    
    bool connect(std::string path, speed_t baudRate = B9600);
    void disconnect();
    
    ssize_t read(void *data, std::size_t size);
    ssize_t write(const void *data, std::size_t size);
    
    template<typename ContiguousContainerType>
    ssize_t read(ContiguousContainerType &buffer) {
        return read(buffer.data(), buffer.size());
    }
    
    template<typename ContiguousContainerType>
    ssize_t write(const ContiguousContainerType &buffer) {
        return write(buffer.data(), buffer.size());
    }
    
private:
    static bool validatePath(std::string &path);
    static void sysError(const std::string &msg);
    
    const bool blocking;
    int fd;
    struct termios origAttrs;
    
};


END_NAMESPACE_MW


#endif /* !defined(MWorksCore_SerialPort_hpp) */




























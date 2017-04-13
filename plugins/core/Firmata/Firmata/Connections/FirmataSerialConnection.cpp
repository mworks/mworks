//
//  FirmataSerialConnection.cpp
//  Firmata
//
//  Created by Christopher Stawarz on 4/13/17.
//  Copyright © 2017 The MWorks Project. All rights reserved.
//

#include "FirmataSerialConnection.hpp"


BEGIN_NAMESPACE_MW


FirmataSerialConnection::FirmataSerialConnection(const std::string &path) :
    path(path)
{ }


FirmataSerialConnection::~FirmataSerialConnection() {
    disconnect();
}


bool FirmataSerialConnection::connect() {
    return serialPort.connect(path, B57600);
}


void FirmataSerialConnection::disconnect() {
    serialPort.disconnect();
}


ssize_t FirmataSerialConnection::read(std::uint8_t &data, std::size_t size) {
    return serialPort.read(&data, size);
}


bool FirmataSerialConnection::write(const std::vector<std::uint8_t> &data) {
    return (-1 != serialPort.write(data));
}


END_NAMESPACE_MW



























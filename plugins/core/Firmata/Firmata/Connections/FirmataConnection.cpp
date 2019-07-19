//
//  FirmataConnection.cpp
//  Firmata
//
//  Created by Christopher Stawarz on 4/13/17.
//  Copyright © 2017 The MWorks Project. All rights reserved.
//

#include "FirmataConnection.hpp"

#if TARGET_OS_OSX
#  include "FirmataSerialConnection.hpp"
#endif
#include "FirmataBluetoothLEConnection.hpp"


BEGIN_NAMESPACE_MW


std::unique_ptr<FirmataConnection> FirmataConnection::create(const ParameterValue &serialPortPath,
                                                             const ParameterValue &bluetoothLocalName)
{
    if (bluetoothLocalName.empty()) {
#if TARGET_OS_OSX
        std::string path;
        if (!(serialPortPath.empty())) {
            path = variableOrText(serialPortPath)->getValue().getString();
        }
        return std::unique_ptr<FirmataConnection>(new FirmataSerialConnection(path));
#else
        throw SimpleException(M_IODEVICE_MESSAGE_DOMAIN, "Connection via serial port is not supported on this OS");
#endif
    }
    const auto localName = variableOrText(bluetoothLocalName)->getValue().getString();
    return std::unique_ptr<FirmataConnection>(new FirmataBluetoothLEConnection(localName));
}


END_NAMESPACE_MW

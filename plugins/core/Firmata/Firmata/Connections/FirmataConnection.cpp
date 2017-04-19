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


std::unique_ptr<FirmataConnection> FirmataConnection::create(const std::string &serialPortPath,
                                                             const std::string &bluetoothLocalName)
{
    if (bluetoothLocalName.empty()) {
#if TARGET_OS_OSX
        return std::unique_ptr<FirmataConnection>(new FirmataSerialConnection(serialPortPath));
#else
        throw SimpleException(M_IODEVICE_MESSAGE_DOMAIN, "Connection via serial port is not supported on this OS");
#endif
    }
    return std::unique_ptr<FirmataConnection>(new FirmataBluetoothLEConnection(bluetoothLocalName));
}


END_NAMESPACE_MW

//
//  FirmataConnection.cpp
//  Firmata
//
//  Created by Christopher Stawarz on 4/13/17.
//  Copyright © 2017 The MWorks Project. All rights reserved.
//

#include "FirmataConnection.hpp"

#include "FirmataSerialConnection.hpp"
#include "FirmataBluetoothLEConnection.hpp"


BEGIN_NAMESPACE_MW


std::unique_ptr<FirmataConnection> FirmataConnection::create(const std::string &serialPortPath,
                                                             const std::string &bluetoothLocalName)
{
    if (bluetoothLocalName.empty()) {
        return std::unique_ptr<FirmataConnection>(new FirmataSerialConnection(serialPortPath));
    }
    return std::unique_ptr<FirmataConnection>(new FirmataBluetoothLEConnection(bluetoothLocalName));
}


END_NAMESPACE_MW

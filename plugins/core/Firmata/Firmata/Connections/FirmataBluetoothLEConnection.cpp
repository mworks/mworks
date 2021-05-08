//
//  FirmataBluetoothLEConnection.cpp
//  Firmata
//
//  Created by Christopher Stawarz on 4/13/17.
//  Copyright © 2017 The MWorks Project. All rights reserved.
//

#include "FirmataBluetoothLEConnection.hpp"

//
// NOTE: This code assumes the Bluetooth LE device implements the Nordic UART Service:
//
//   https://thejeshgn.com/2016/10/01/uart-over-bluetooth-low-energy/
//   http://infocenter.nordicsemi.com/index.jsp?topic=%2Fcom.nordic.infocenter.sdk51.v9.0.0%2Fble_sdk_app_nus_eval.html&cp=4_0_5_4_4_17
//
// The Arduino Firmata library provides a compatible implementation:
//
//   https://github.com/firmata/arduino/tree/master/examples/StandardFirmataBLE
//

#define UART_SERVICE_UUID      @"6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define RX_CHARACTERISTIC_UUID @"6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define TX_CHARACTERISTIC_UUID @"6E400003-B5A3-F393-E0A9-E50E24DCCA9E"


@interface MWKFirmataBluetoothLEDelegate : NSObject <CBCentralManagerDelegate, CBPeripheralDelegate>

@property(nonatomic, readonly) CBCentralManager *centralManager;
@property(nonatomic, strong) CBPeripheral *peripheral;
@property(nonatomic, strong) CBService *service;
@property(nonatomic, strong) CBCharacteristic *rxCharacteristic;
@property(nonatomic, strong) CBCharacteristic *txCharacteristic;

@end


@implementation MWKFirmataBluetoothLEDelegate {
    mw::FirmataBluetoothLEConnection *connection;
    mw::FirmataBluetoothLEConnection::NotifyCallback notifyCallback;
    mw::FirmataBluetoothLEConnection::DataReceivedCallback dataReceivedCallback;
    mw::FirmataBluetoothLEConnection::DisconnectedCallback disconnectedCallback;
    dispatch_queue_t queue;
}


- (instancetype)initWithConnection:(mw::FirmataBluetoothLEConnection *)_connection
                    notifyCallback:(mw::FirmataBluetoothLEConnection::NotifyCallback)_notifyCallback
              dataReceivedCallback:(mw::FirmataBluetoothLEConnection::DataReceivedCallback)_dataReceivedCallback
              disconnectedCallback:(mw::FirmataBluetoothLEConnection::DisconnectedCallback)_disconnectedCallback
{
    self = [super init];
    if (self) {
        connection = _connection;
        notifyCallback = _notifyCallback;
        dataReceivedCallback = _dataReceivedCallback;
        disconnectedCallback = _disconnectedCallback;
        queue = dispatch_queue_create(nullptr, dispatch_queue_attr_make_with_qos_class(DISPATCH_QUEUE_SERIAL,
                                                                                       QOS_CLASS_USER_INITIATED,
                                                                                       0));
        _centralManager = [[CBCentralManager alloc] initWithDelegate:self queue:queue];
    }
    return self;
}


- (void)centralManagerDidUpdateState:(CBCentralManager *)central {
    if (central.state == CBManagerStatePoweredOn) {
        notifyCallback(self, connection);
    }
}


- (void)centralManager:(CBCentralManager *)central
 didDiscoverPeripheral:(CBPeripheral *)peripheral
     advertisementData:(NSDictionary<NSString *, id> *)advertisementData
                  RSSI:(NSNumber *)RSSI
{
    if (!self.peripheral) {
        NSString *advertisedLocalName = advertisementData[CBAdvertisementDataLocalNameKey];
        if (advertisedLocalName &&
            [advertisedLocalName isEqualToString:@(connection->getLocalName().c_str())])
        {
            self.peripheral = peripheral;
            [central stopScan];
            notifyCallback(self, connection);
        }
    }
}
    
    
- (void)centralManager:(CBCentralManager *)central
didFailToConnectPeripheral:(CBPeripheral *)peripheral
                 error:(NSError *)error
{
    mw::merror(mw::M_IODEVICE_MESSAGE_DOMAIN,
               "Failed to connect to Bluetooth device: %s",
               error.localizedDescription.UTF8String);
}


- (void)centralManager:(CBCentralManager *)central didConnectPeripheral:(CBPeripheral *)peripheral {
    peripheral.delegate = self;
    notifyCallback(self, connection);
}


- (void)peripheral:(CBPeripheral *)peripheral didDiscoverServices:(NSError *)error {
    if (error) {
        mw::merror(mw::M_IODEVICE_MESSAGE_DOMAIN,
                   "Failed to discover services of Bluetooth device: %s",
                   error.localizedDescription.UTF8String);
    } else if (peripheral.services.count > 0) {
        self.service = peripheral.services[0];
        notifyCallback(self, connection);
    }
}


- (void)peripheral:(CBPeripheral *)peripheral
didDiscoverCharacteristicsForService:(CBService *)service
             error:(NSError *)error
{
    if (error) {
        mw::merror(mw::M_IODEVICE_MESSAGE_DOMAIN,
                   "Failed to discover service characteristics of Bluetooth device: %s",
                   error.localizedDescription.UTF8String);
    } else {
        for (CBCharacteristic *characteristic in service.characteristics) {
            if (!self.rxCharacteristic && [characteristic.UUID.UUIDString isEqualToString:RX_CHARACTERISTIC_UUID]) {
                self.rxCharacteristic = characteristic;
            } else if (!self.txCharacteristic && [characteristic.UUID.UUIDString isEqualToString:TX_CHARACTERISTIC_UUID]) {
                self.txCharacteristic = characteristic;
            }
            
            if (self.rxCharacteristic && self.txCharacteristic) {
                notifyCallback(self, connection);
                break;
            }
        }
    }
}


- (void)peripheral:(CBPeripheral *)peripheral
didUpdateNotificationStateForCharacteristic:(CBCharacteristic *)characteristic
             error:(NSError *)error
{
    if (error) {
        mw::merror(mw::M_IODEVICE_MESSAGE_DOMAIN,
                   "Failed to subscribe to updates from Bluetooth device: %s",
                   error.localizedDescription.UTF8String);
    } else {
        notifyCallback(self, connection);
    }
}


- (void)peripheral:(CBPeripheral *)peripheral
didUpdateValueForCharacteristic:(CBCharacteristic *)characteristic
             error:(NSError *)error
{
    if (error) {
        mw::merror(mw::M_IODEVICE_MESSAGE_DOMAIN,
                   "Failed to receive updated service characteristic value from Bluetooth device: %s",
                   error.localizedDescription.UTF8String);
    } else {
        NSData *data = characteristic.value;
        dataReceivedCallback(self, connection, static_cast<const std::uint8_t *>(data.bytes), data.length);
    }
}


- (void)peripheral:(CBPeripheral *)peripheral
didWriteValueForCharacteristic:(CBCharacteristic *)characteristic
             error:(NSError *)error
{
    if (error) {
        mw::merror(mw::M_IODEVICE_MESSAGE_DOMAIN,
                   "Failed to send updated service characteristic value to Bluetooth device: %s",
                   error.localizedDescription.UTF8String);
    } else {
        notifyCallback(self, connection);
    }
}


- (void)centralManager:(CBCentralManager *)central
didDisconnectPeripheral:(CBPeripheral *)peripheral
                 error:(NSError *)error
{
    if (error) {
        mw::merror(mw::M_IODEVICE_MESSAGE_DOMAIN,
                   "Disconnected from Bluetooth device: %s",
                   error.localizedDescription.UTF8String);
        disconnectedCallback(self, connection);
    } else {
        notifyCallback(self, connection);
    }
}


@end


BEGIN_NAMESPACE_MW


void FirmataBluetoothLEConnection::notify(MWKFirmataBluetoothLEDelegate *delegate,
                                          FirmataBluetoothLEConnection *connection)
{
    {
        unique_lock lock(connection->mutex);
        if (delegate != connection->delegate) {
            // Ignore notifications from previous delegates
            return;
        }
        connection->wasNotified = true;
    }
    connection->condition.notify_all();
}


void FirmataBluetoothLEConnection::dataReceived(MWKFirmataBluetoothLEDelegate *delegate,
                                                FirmataBluetoothLEConnection *connection,
                                                const std::uint8_t *data,
                                                std::size_t size)
{
    {
        unique_lock lock(connection->mutex);
        if (delegate != connection->delegate) {
            // Ignore received data from previous delegates
            return;
        }
        connection->receivedData.insert(connection->receivedData.end(), data, data + size);
    }
    connection->condition.notify_all();
}


void FirmataBluetoothLEConnection::disconnected(MWKFirmataBluetoothLEDelegate *delegate,
                                                FirmataBluetoothLEConnection *connection)
{
    unique_lock lock(connection->mutex);
    if (delegate != connection->delegate) {
        // Ignore disconnections from previous delegates
        return;
    }
    connection->connected = false;
}


FirmataBluetoothLEConnection::FirmataBluetoothLEConnection(FirmataConnectionClient &client,
                                                           const std::string &localName) :
    FirmataConnection(client),
    localName(localName),
    delegate(nil),
    wasNotified(false),
    connected(false)
{ }


bool FirmataBluetoothLEConnection::connect() {
    unique_lock lock(mutex);
    
    @autoreleasepool {
        delegate = [[MWKFirmataBluetoothLEDelegate alloc] initWithConnection:this
                                                              notifyCallback:&notify
                                                        dataReceivedCallback:&dataReceived
                                                        disconnectedCallback:&disconnected];
        if (!wait(lock)) {
            merror(M_IODEVICE_MESSAGE_DOMAIN, "Bluetooth is not available");
            return false;
        }
        
        [delegate.centralManager scanForPeripheralsWithServices:@[[CBUUID UUIDWithString:UART_SERVICE_UUID]] options:nil];
        if (!wait(lock)) {
            merror(M_IODEVICE_MESSAGE_DOMAIN, "Cannot discover Bluetooth device");
            return false;
        }
        
        [delegate.centralManager connectPeripheral:delegate.peripheral options:nil];
        if (!wait(lock)) {
            merror(M_IODEVICE_MESSAGE_DOMAIN, "Cannot connect to Bluetooth device");
            return false;
        }
        
        [delegate.peripheral discoverServices:@[[CBUUID UUIDWithString:UART_SERVICE_UUID]]];
        if (!wait(lock)) {
            merror(M_IODEVICE_MESSAGE_DOMAIN, "Bluetooth device does not provide the required service");
            return false;
        }
        
        [delegate.peripheral discoverCharacteristics:@[[CBUUID UUIDWithString:RX_CHARACTERISTIC_UUID],
                                                       [CBUUID UUIDWithString:TX_CHARACTERISTIC_UUID]]
                                          forService:delegate.service];
        if (!wait(lock)) {
            merror(M_IODEVICE_MESSAGE_DOMAIN, "Bluetooth device does not provide the required service characteristics");
            return false;
        }
        
        [delegate.peripheral setNotifyValue:YES forCharacteristic:delegate.txCharacteristic];
        if (!wait(lock)) {
            merror(M_IODEVICE_MESSAGE_DOMAIN, "Cannot subscribe to updates from Bluetooth device");
            return false;
        }
        
        connected = true;
        
        return true;
    }
}


void FirmataBluetoothLEConnection::disconnect() {
    unique_lock lock(mutex);
    @autoreleasepool {
        if (delegate) {
            if (delegate.peripheral &&
                (delegate.peripheral.state == CBPeripheralStateConnecting ||
                 delegate.peripheral.state == CBPeripheralStateConnected))
            {
                [delegate.centralManager cancelPeripheralConnection:delegate.peripheral];
                if (!wait(lock)) {
                    merror(M_IODEVICE_MESSAGE_DOMAIN, "Cannot disconnect from Bluetooth device");
                }
            }
            delegate = nil;
            receivedData.clear();
            wasNotified = false;
            connected = false;
        }
    }
}


ssize_t FirmataBluetoothLEConnection::read(std::uint8_t &data, std::size_t size) {
    unique_lock lock(mutex);
    if (!connected) {
        merror(M_IODEVICE_MESSAGE_DOMAIN, "Cannot read from Bluetooth device: Not connected");
        return -1;
    }
    if (receivedData.empty()) {
        if (!condition.wait_for(lock, std::chrono::milliseconds(500), [this]() { return !(receivedData.empty()); })) {
            return 0;
        }
    }
    const auto numBytes = std::min(size, receivedData.size());
    std::copy_n(receivedData.begin(), numBytes, &data);
    receivedData.erase(receivedData.begin(), receivedData.begin() + numBytes);
    return numBytes;
}


bool FirmataBluetoothLEConnection::write(const std::vector<std::uint8_t> &data) {
    unique_lock lock(mutex);
    if (!connected) {
        merror(M_IODEVICE_MESSAGE_DOMAIN, "Cannot write to Bluetooth device: Not connected");
        return false;
    }
    @autoreleasepool {
        NSData *value = [NSData dataWithBytes:data.data() length:data.size()];
        if (delegate.rxCharacteristic.properties & CBCharacteristicPropertyWrite) {
            // rxCharacteristic supports write with response, so wait for acknowledgement
            // after writing
            [delegate.peripheral writeValue:value
                          forCharacteristic:delegate.rxCharacteristic
                                       type:CBCharacteristicWriteWithResponse];
            if (!wait(lock, std::chrono::milliseconds(500))) {
                merror(M_IODEVICE_MESSAGE_DOMAIN, "Cannot write to Bluetooth device");
                return false;
            }
        } else {
            // rxCharacteristic doesn't support write with response, so don't wait after
            // writing
            [delegate.peripheral writeValue:value
                          forCharacteristic:delegate.rxCharacteristic
                                       type:CBCharacteristicWriteWithoutResponse];
        }
        return true;
    }
}


inline bool FirmataBluetoothLEConnection::wait(unique_lock &lock, std::chrono::milliseconds timeout) {
    wasNotified = false;
    return condition.wait_for(lock, timeout, [this]() { return wasNotified; });
}


END_NAMESPACE_MW

//
//  QCUALORDevice.cpp
//  QCUALOR
//
//  Created by Christopher Stawarz on 3/22/22.
//

#include "QCUALORDevice.hpp"


BEGIN_NAMESPACE_MW


const std::string QCUALORDevice::SERIAL_PORT("serial_port");
const std::string QCUALORDevice::MODE_1("mode_1");
const std::string QCUALORDevice::MODE_2("mode_2");
const std::string QCUALORDevice::MODE_3("mode_3");
const std::string QCUALORDevice::MODE_4("mode_4");
const std::string QCUALORDevice::MODULATION_FREQUENCY("modulation_frequency");
const std::string QCUALORDevice::GAIN_1("gain_1");
const std::string QCUALORDevice::GAIN_2("gain_2");
const std::string QCUALORDevice::GAIN_3("gain_3");
const std::string QCUALORDevice::GAIN_4("gain_4");
const std::string QCUALORDevice::MAX_GAIN("max_gain");
const std::string QCUALORDevice::MEG_STATE("meg_state");
const std::string QCUALORDevice::LOG_COMMANDS("log_commands");


void QCUALORDevice::describeComponent(ComponentInfo &info) {
    IODevice::describeComponent(info);
    
    info.setSignature("iodevice/qcualor");
    
    info.addParameter(SERIAL_PORT, false);
    info.addParameter(MODE_1, false);
    info.addParameter(MODE_2, false);
    info.addParameter(MODE_3, false);
    info.addParameter(MODE_4, false);
    info.addParameter(MODULATION_FREQUENCY, "0.5");
    info.addParameter(GAIN_1, "0.0");
    info.addParameter(GAIN_2, "0.0");
    info.addParameter(GAIN_3, "0.0");
    info.addParameter(GAIN_4, "0.0");
    info.addParameter(MAX_GAIN, "0.8");
    info.addParameter(MEG_STATE, false);
    info.addParameter(LOG_COMMANDS, "NO");
}


QCUALORDevice::QCUALORDevice(const ParameterValueMap &parameters) :
    IODevice(parameters),
    serialPortPath(optionalVariableOrText(parameters[SERIAL_PORT])),
    mode1(optionalVariableOrText(parameters[MODE_1])),
    mode2(optionalVariableOrText(parameters[MODE_2])),
    mode3(optionalVariableOrText(parameters[MODE_3])),
    mode4(optionalVariableOrText(parameters[MODE_4])),
    modulationFrequency(parameters[MODULATION_FREQUENCY]),
    gain1(parameters[GAIN_1]),
    gain2(parameters[GAIN_2]),
    gain3(parameters[GAIN_3]),
    gain4(parameters[GAIN_4]),
    gainMax(parameters[MAX_GAIN]),
    megState(optionalVariable(parameters[MEG_STATE])),
    logCommands(parameters[LOG_COMMANDS])
{
    if (gainMax < 0.0 || gainMax > 1.0) {
        throw SimpleException(M_IODEVICE_MESSAGE_DOMAIN, boost::format("%1% must be between 0 and 1") % MAX_GAIN);
    }
}


bool QCUALORDevice::initialize() {
    std::string path;
    if (serialPortPath) {
        path = serialPortPath->getValue().getString();
    }
    return serialPort.connect(path, B115200);
}


bool QCUALORDevice::startDeviceIO() {
    lock_guard lock(mutex);
    
    if (megState && megState->getValue().getBool()) {
        merror(M_IODEVICE_MESSAGE_DOMAIN,
               "Cannot start QCUALOR device: modulation enable gate (MEG) state is already high/active");
        return false;
    }
    
    constexpr double frequencyMin = 0.5;
    constexpr double frequencyMax = 500.0;
    auto frequency = modulationFrequency->getValue().getFloat();
    if (frequency < frequencyMin || frequency > frequencyMax) {
        merror(M_IODEVICE_MESSAGE_DOMAIN,
               "QCUALOR modulation frequency must be between %g and %g",
               frequencyMin,
               frequencyMax);
        return false;
    }
    std::uint8_t frequencyIndex = std::round(255.0
                                             * std::log(frequency / frequencyMin)
                                             / std::log(frequencyMax / frequencyMin));
    
    std::array<Message, 4> commands;
    if (!prepareCommand(CHANNEL_1, mode1, frequencyIndex, gain1, commands[0]) ||
        !prepareCommand(CHANNEL_2, mode2, frequencyIndex, gain2, commands[1]) ||
        !prepareCommand(CHANNEL_3, mode3, frequencyIndex, gain3, commands[2]) ||
        !prepareCommand(CHANNEL_4, mode4, frequencyIndex, gain4, commands[3]))
    {
        return false;
    }
    
    for (auto &command : commands) {
        if (!sendCommand(command)) {
            return false;
        }
    }
    
    if (megState) {
        megState->setValue(Datum(true));
    }
    
    return true;
}


bool QCUALORDevice::stopDeviceIO() {
    lock_guard lock(mutex);
    
    if (megState && megState->getValue().getBool()) {
        megState->setValue(Datum(false));
    }
    
    // Configure all active channels to "OFF"
    if (!deactivateAllChannels()) {
        return false;
    }
    
    return true;
}


QCUALORDevice::Message::Message(std::uint8_t channel,
                                std::uint8_t mode,
                                std::uint8_t frequencyIndex,
                                std::uint16_t gain)
{
    bytes[0] = channel | mode;
    bytes[1] = frequencyIndex;
    boost::endian::store_little_u16(&(bytes[2]), gain);
}


bool QCUALORDevice::Message::send(SerialPort &serialPort) const {
    std::size_t bytesSent = 0;
    do {
        auto result = serialPort.write(bytes.data() + bytesSent, bytes.size() - bytesSent);
        if (-1 == result) {
            merror(M_IODEVICE_MESSAGE_DOMAIN, "Cannot send configuration command to QCUALOR device");
            return false;
        }
        bytesSent += result;
    } while (bytesSent < bytes.size());
    
    return true;
}


bool QCUALORDevice::Message::receive(SerialPort &serialPort) {
    std::size_t bytesReceived = 0;
    do {
        auto result = serialPort.read(bytes.data() + bytesReceived, bytes.size() - bytesReceived);
        if (-1 == result) {
            merror(M_IODEVICE_MESSAGE_DOMAIN, "Cannot receive configuration response from QCUALOR device");
            return false;
        }
        if (0 == result) {
            if (0 == bytesReceived) {
                merror(M_IODEVICE_MESSAGE_DOMAIN, "QCUALOR device did not respond to configuration command");
            } else {
                merror(M_IODEVICE_MESSAGE_DOMAIN,
                       "Received incomplete configuration response (%ld of %ld bytes) from QCUALOR device",
                       bytesReceived,
                       bytes.size());
            }
            return false;
        }
        bytesReceived += result;
    } while (bytesReceived < bytes.size());
    
    return true;
}


std::string QCUALORDevice::Message::toString() const {
    std::ostringstream os;
    os << "0b" << std::bitset<8>(bytes[0]);
    os << ' ' << std::setw(3) << int(bytes[1]);
    os << " 0x" << std::hex << std::setfill('0') << std::setw(2) << int(bytes[2]);
    os << " 0x" << std::hex << std::setfill('0') << std::setw(2) << int(bytes[3]);
    return os.str();
}


bool QCUALORDevice::prepareCommand(std::uint8_t channel,
                                   const VariablePtr &modeVar,
                                   std::uint8_t frequencyIndex,
                                   const VariablePtr &gainVar,
                                   Message &command) const
{
    std::uint8_t mode = MODE_OFF;
    if (modeVar) {
        auto modeName = boost::algorithm::to_lower_copy(modeVar->getValue().getString());
        if (modeName == "continuous") {
            mode = MODE_CW;
        } else if (modeName == "sinusoidal") {
            mode = MODE_SW;
        } else if (modeName == "inverse_sinusoidal") {
            mode = MODE_ISW;
        } else if (modeName == "square") {
            mode = MODE_SQW;
        } else if (modeName != "off") {
            merror(M_IODEVICE_MESSAGE_DOMAIN, "Invalid QCUALOR channel mode: \"%s\"", modeName.c_str());
            return false;
        }
    }
    
    auto gain = gainVar->getValue().getFloat();
    if (gain < 0.0 || gain > gainMax) {
        merror(M_IODEVICE_MESSAGE_DOMAIN, "QCUALOR channel gain must be between 0 and %g", gainMax);
        return false;
    }
    
    command = Message(channel,
                      mode,
                      frequencyIndex,
                      std::round(gain * double(std::numeric_limits<std::uint16_t>::max())));
    
    return true;
}


bool QCUALORDevice::sendCommand(const Message &command) {
    // Assume as-yet-unconfigured channels are active, so that the first
    // configuration command for the channel always executes
    auto &isActive = active.emplace(command.getChannel(), true).first->second;
    const auto shouldActivate = (command.getMode() != MODE_OFF);
    
    /*
    if (!(isActive || shouldActivate)) {
        // Channel is being set to "OFF", but it's already inactive, so there's
        // nothing to do
        return true;
    }
     */
    
    // Send command
    if (!command.send(serialPort)) {
        return false;
    }
    if (logCommands->getValue().getBool()) {
        mprintf(M_IODEVICE_MESSAGE_DOMAIN, "SEND: %s", command.toString().c_str());
    }
    
    Message response;
    
    // Receive response
    if (!response.receive(serialPort)) {
        return false;
    }
    if (logCommands->getValue().getBool()) {
        mprintf(M_IODEVICE_MESSAGE_DOMAIN, "RECV: %s", response.toString().c_str());
    }
    
    // Check that response is equivalent to command
    if (!response.isEqual(command)) {
        merror(M_IODEVICE_MESSAGE_DOMAIN, "Received invalid configuration response from QCUALOR device");
        return false;
    }
    
    // Update "active" flag for configured channel
    isActive = shouldActivate;
    
    return true;
}


bool QCUALORDevice::deactivateAllChannels() {
    return (sendCommand(Message(CHANNEL_1)) &&
            sendCommand(Message(CHANNEL_2)) &&
            sendCommand(Message(CHANNEL_3)) &&
            sendCommand(Message(CHANNEL_4)));
}


END_NAMESPACE_MW

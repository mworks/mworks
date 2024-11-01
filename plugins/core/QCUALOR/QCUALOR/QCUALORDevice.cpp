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
    if (!prepareCommand(CHANNEL_1, mode1, frequencyIndex, gain1, commands.at(0)) ||
        !prepareCommand(CHANNEL_2, mode2, frequencyIndex, gain2, commands.at(1)) ||
        !prepareCommand(CHANNEL_3, mode3, frequencyIndex, gain3, commands.at(2)) ||
        !prepareCommand(CHANNEL_4, mode4, frequencyIndex, gain4, commands.at(3)))
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
    
    if (!sendCommand(prepareCommand(CHANNEL_1)) ||
        !sendCommand(prepareCommand(CHANNEL_2)) ||
        !sendCommand(prepareCommand(CHANNEL_3)) ||
        !sendCommand(prepareCommand(CHANNEL_4)))
    {
        return false;
    }
    
    return true;
}


std::string QCUALORDevice::messageToHex(const Message &message) {
    std::ostringstream os;
    os << "\t0b" << std::bitset<8>(message.at(0));
    os << '\t' << int(message.at(1));
    os << "\t0x" << std::hex << std::setfill('0') << std::setw(2) << int(message.at(2));
    os << "\t0x" << std::hex << std::setfill('0') << std::setw(2) << int(message.at(3));
    return os.str();
}


auto QCUALORDevice::prepareCommand(std::uint8_t channel,
                                   std::uint8_t mode,
                                   std::uint8_t frequencyIndex,
                                   std::uint16_t gain) -> Message
{
    Message command;
    command.at(0) = channel | mode;
    command.at(1) = frequencyIndex;
    boost::endian::store_little_u16(&(command.at(2)), gain);
    return command;
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
    
    command = prepareCommand(channel,
                             mode,
                             frequencyIndex,
                             std::round(gain * double(std::numeric_limits<std::uint16_t>::max())));
    
    return true;
}


bool QCUALORDevice::sendCommand(const Message &command) {
    // Send command
    {
        std::size_t bytesSent = 0;
        do {
            auto result = serialPort.write(command.data() + bytesSent, command.size() - bytesSent);
            if (-1 == result) {
                merror(M_IODEVICE_MESSAGE_DOMAIN, "Cannot send configuration command to QCUALOR device");
                return false;
            }
            bytesSent += result;
        } while (bytesSent < command.size());
        
        if (logCommands->getValue().getBool()) {
            mprintf(M_IODEVICE_MESSAGE_DOMAIN, "SEND: %s", messageToHex(command).c_str());
        }
    }
    
    Message response;
    
    // Receive response
    {
        std::size_t bytesReceived = 0;
        do {
            auto result = serialPort.read(response.data() + bytesReceived, response.size() - bytesReceived);
            if (-1 == result) {
                merror(M_IODEVICE_MESSAGE_DOMAIN, "Cannot receive configuration response from QCUALOR device");
                return false;
            }
            if (0 == result) {
                merror(M_IODEVICE_MESSAGE_DOMAIN, "QCUALOR device did not respond to configuration command");
                return false;
            }
            bytesReceived += result;
        } while (bytesReceived < response.size());
        
        if (logCommands->getValue().getBool()) {
            mprintf(M_IODEVICE_MESSAGE_DOMAIN, "RECV: %s", messageToHex(response).c_str());
        }
    }
    
    // Check that response is equivalent to command
    if (response != command) {
        merror(M_IODEVICE_MESSAGE_DOMAIN, "Received invalid configuration response from QCUALOR device");
        return false;
    }
    
    return true;
}


END_NAMESPACE_MW

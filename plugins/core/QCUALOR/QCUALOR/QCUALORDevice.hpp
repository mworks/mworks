//
//  QCUALORDevice.hpp
//  QCUALOR
//
//  Created by Christopher Stawarz on 3/22/22.
//

#ifndef QCUALORDevice_hpp
#define QCUALORDevice_hpp


BEGIN_NAMESPACE_MW


class QCUALORDevice : public IODevice, boost::noncopyable {
    
public:
    static const std::string SERIAL_PORT;
    static const std::string MODE_1;
    static const std::string MODE_2;
    static const std::string MODE_3;
    static const std::string MODE_4;
    static const std::string MODULATION_FREQUENCY;
    static const std::string GAIN_1;
    static const std::string GAIN_2;
    static const std::string GAIN_3;
    static const std::string GAIN_4;
    static const std::string MAX_GAIN;
    static const std::string MEG_STATE;
    static const std::string LOG_COMMANDS;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit QCUALORDevice(const ParameterValueMap &parameters);
    
    bool initialize() override;
    bool startDeviceIO() override;
    bool stopDeviceIO() override;
    
private:
    enum {
        CHANNEL_MASK = 0b11000000,
        CHANNEL_1    = 0b00000000,
        CHANNEL_2    = 0b01000000,
        CHANNEL_3    = 0b10000000,
        CHANNEL_4    = 0b11000000,
        
        MODE_MASK = 0b111,
        MODE_OFF  = 0b000,
        MODE_CW   = 0b001,
        MODE_SW   = 0b010,
        MODE_ISW  = 0b011,
        MODE_SQW  = 0b100
    };
    
    struct Message {
        constexpr Message() : bytes{ 0, 0, 0, 0 } { }
        
        explicit Message(std::uint8_t channel,
                         std::uint8_t mode = MODE_OFF,
                         std::uint8_t frequencyIndex = 0,
                         std::uint16_t gain = 0);
        
        std::uint8_t getChannel() const { return (bytes[0] & CHANNEL_MASK); }
        std::uint8_t getMode() const { return (bytes[0] & MODE_MASK); }
        
        bool send(SerialPort &serialPort) const;
        bool receive(SerialPort &serialPort);
        
        bool isEqual(const Message &other) const { return (bytes == other.bytes); }
        std::string toString() const;
        
    private:
        std::array<std::uint8_t, 4> bytes;
    };
    
    bool prepareCommand(std::uint8_t channel,
                        const VariablePtr &modeVar,
                        std::uint8_t frequencyIndex,
                        const VariablePtr &gainVar,
                        Message &command) const;
    
    bool sendCommand(const Message &command);
    bool deactivateAllChannels();
    
    const VariablePtr serialPortPath;
    const VariablePtr mode1;
    const VariablePtr mode2;
    const VariablePtr mode3;
    const VariablePtr mode4;
    const VariablePtr modulationFrequency;
    const VariablePtr gain1;
    const VariablePtr gain2;
    const VariablePtr gain3;
    const VariablePtr gain4;
    const double gainMax;
    const VariablePtr megState;
    const VariablePtr logCommands;
    
    SerialPort serialPort;
    
    std::map<std::uint8_t, bool> active;
    
    using lock_guard = std::lock_guard<std::mutex>;
    lock_guard::mutex_type mutex;
    
};


END_NAMESPACE_MW


#endif /* QCUALORDevice_hpp */

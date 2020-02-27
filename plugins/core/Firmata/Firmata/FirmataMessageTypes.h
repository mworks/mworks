//
//  FirmataMessageTypes.h
//  Firmata
//
//  Created by Christopher Stawarz on 2/27/20.
//  Copyright © 2020 The MWorks Project. All rights reserved.
//

#ifndef FirmataMessageTypes_h
#define FirmataMessageTypes_h


enum {
    DIGITAL_MESSAGE         = 0x90, // send data for a digital port (collection of 8 pins)
    ANALOG_MESSAGE          = 0xE0, // send data for an analog pin (or PWM)
    REPORT_ANALOG           = 0xC0, // enable analog input by pin #
    REPORT_DIGITAL          = 0xD0, // enable digital input by port pair
    
    SET_PIN_MODE            = 0xF4, // set a pin to INPUT/OUTPUT/PWM/etc
    SET_DIGITAL_PIN_VALUE   = 0xF5, // set value of an individual digital pin
    
    REPORT_VERSION          = 0xF9, // report protocol version
    SYSTEM_RESET            = 0xFF, // reset from MIDI
    
    START_SYSEX             = 0xF0, // start a MIDI Sysex message
    END_SYSEX               = 0xF7, // end a MIDI Sysex message
    
    SERVO_CONFIG            = 0x70, // set minPulse, maxPulse
    REPORT_FIRMWARE         = 0x79, // report name and version of the firmware
    EXTENDED_ANALOG         = 0x6F, // analog write (PWM, Servo, etc) to any pin
    CAPABILITY_QUERY        = 0x6B, // ask for supported modes and resolution of all pins
    CAPABILITY_RESPONSE     = 0x6C, // reply with supported modes and resolution
    ANALOG_MAPPING_QUERY    = 0x69, // ask for mapping of analog to pin numbers
    ANALOG_MAPPING_RESPONSE = 0x6A, // reply with mapping info
    SAMPLING_INTERVAL       = 0x7A  // set the poll rate of the main loop
};


#endif /* FirmataMessageTypes_h */

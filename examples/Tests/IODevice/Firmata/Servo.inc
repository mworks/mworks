//
// NOTE: This file must be included by a file that defines the macro
// "bluetooth_local_name"
//

//
// The values of min_pulse_width and max_pulse_width used below are for the
// SpringRC SM-S2309S (http://www.springrc.com/en/pd.jsp?id=23).  Note that the
// range of motion for this servo is only 120 degrees.
//

var value = 0

firmata arduino (
    bluetooth_local_name = bluetooth_local_name
    ) {
    firmata_servo (
        pin_number = 5
        value = value
        min_pulse_width = 900
        max_pulse_width = 2100
        )
}


protocol {
    start_io_device (arduino)
    report ('Servo should be at 0 degrees')
    wait (2s)

    value = 90
    report ('Servo should be at 90 degrees')
    wait (2s)

    value = 180
    report ('Servo should be at 180 degrees')
    wait (2s)

    // Invalid values
    value = -0.1
    value = 180.1

    stop_io_device (arduino)
    // Reset value to 0, so that start_io_device resets the servo position on
    // the next run
    value = 0
}

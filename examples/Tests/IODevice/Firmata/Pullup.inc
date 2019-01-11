//
// NOTE: This file must be included by a file that defines the macro
// "bluetooth_local_name"
//

//
// NOTE: This test is valid only if the input pin (A3) is *not*
// connected
//


var value = false {
    report ('value = $value')
}


firmata firmata (
    bluetooth_local_name = bluetooth_local_name
    ) {
    firmata_digital_input_pullup (
        pin_number = a3
        value = value
        )
}


protocol {
    block (nsamples = 2) {
        value = false

        report ('Starting device')
        start_io_device (firmata)

        wait (1s)

        assert (value == true)

        report ('Stopping device')
        stop_io_device (firmata)
    }
}

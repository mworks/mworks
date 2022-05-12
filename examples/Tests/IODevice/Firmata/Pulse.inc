%require bluetooth_local_name
%require comm_delay_tolerance
%require duration_tolerance


var output_on_time = 0
var output_duration = 0
var output = false {
    choose {
        when (output) {
            output_on_time = now()
            report ('Output on')
        }
        when (output_on_time != 0) {
            output_duration = now() - output_on_time
            report ('Output off; duration = $(output_duration / 1000) ms')
            output_on_time = 0
        }
    }
}

var input_pulse = 0 {
    if (input_pulse != 0) {
        report ('Input pulse duration = $(input_pulse / 1000) ms')
    }
}

var output_pulse = 0 {
    if (output_pulse != 0) {
        report ('Output pulse duration = $(output_pulse / 1000) ms')
    }
}

var input_on_time = 0
var input_duration = 0
var input = false {
    choose {
        when (input) {
            input_on_time = now()
            report ('Input on')
        }
        when (input_on_time != 0) {
            input_duration = now() - input_on_time
            report ('Input off; duration = $(input_duration / 1000) ms')
            input_on_time = 0
        }
    }
}


firmata firmata (
    bluetooth_local_name = bluetooth_local_name
    autostart = true
    ) {
    firmata_digital_output (
        pin_number = 9
        value = output
        )
    firmata_digital_input_pulse (
        pin_number = a5
        value = input_pulse
        )
    firmata_digital_output_pulse (
        pin_number = 12
        value = output_pulse
        )
    firmata_digital_input (
        pin_number = 13
        value = input
        )
}


%define test_input_pulse (duration_ms)
    output = false
    input_pulse = 0
    wait (comm_delay_tolerance)
    output = true
    wait (duration_ms * 1ms)
    output = false
    wait_for_condition (
        condition = input_pulse != 0
        timeout = comm_delay_tolerance
        )
    assert (abs(output_duration - input_pulse) <= duration_tolerance)
%end

%define test_output_pulse (duration_ms)
    input = false
    input_duration = 0
    wait (comm_delay_tolerance)
    output_pulse = duration_ms * 1ms
    wait_for_condition (
        condition = input
        timeout = comm_delay_tolerance
        )
    wait_for_condition (
        condition = input_duration != 0
        timeout = output_pulse + comm_delay_tolerance
        )
    assert (abs(output_pulse - input_duration) <= duration_tolerance)
%end


protocol {
    block (nsamples = 2) {
        test_input_pulse (disc_rand(75, 125))
        test_input_pulse (disc_rand(400, 600))
        test_input_pulse (disc_rand(900, 1100))
        test_input_pulse (disc_rand(2800, 3200))

        test_output_pulse (disc_rand(75, 125))
        test_output_pulse (disc_rand(400, 600))
        test_output_pulse (disc_rand(900, 1100))
        test_output_pulse (disc_rand(2800, 3200))

        report ('Stopping and restarting device')
        stop_io_device (firmata)
        wait (1s)
        start_io_device (firmata)
    }

    output_pulse = 999us
    output_pulse = 1.5ms
}

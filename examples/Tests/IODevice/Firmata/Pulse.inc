%require bluetooth_local_name
%require comm_delay_tolerance
%require duration_tolerance


var output = false
var output_on_time = 0
var output_duration = 0

var input_pulse = 0

var output_pulse = 0

var input = false
var input_on_time = 0
var input_duration = 0


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
    wait (comm_delay_tolerance)
    input_pulse = 0

    report ('Output on')
    output_on_time = now()
    output = true

    wait ((output_on_time + duration_ms * 1ms) - now())
    output_duration = now() - output_on_time
    output = false
    report ('Output off; duration = $(output_duration / 1000) ms')

    wait_for_condition (
        condition = input_pulse != 0
        timeout = comm_delay_tolerance
        )
    report ('Input pulse duration = $(input_pulse / 1000) ms')

    assert (abs(output_duration - input_pulse) <= duration_tolerance)
%end

%define test_output_pulse (duration_ms)
    input = false

    output_pulse = duration_ms * 1ms
    report ('Output pulse duration = $(output_pulse / 1000) ms')

    wait_for_condition (
        condition = input
        timeout = comm_delay_tolerance
        )
    input_on_time = now()
    report ('Input on')

    wait_for_condition (
        condition = not input
        timeout = output_pulse + comm_delay_tolerance
        )
    input_duration = now() - input_on_time
    report ('Input off; duration = $(input_duration / 1000) ms')

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

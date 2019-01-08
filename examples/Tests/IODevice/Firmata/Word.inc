//
// NOTE: This file must be included by a file that defines the macro
// "bluetooth_local_name"
//


var word = 0 {
    report ('word = $word')
}

var bit0 = false {
    report ('bit0 = $bit0')
}

var bit1 = false {
    report ('bit1 = $bit1')
}

var bit2 = false {
    report ('bit2 = $bit2')
}

var bit3 = false {
    report ('bit3 = $bit3')
}


firmata firmata (
    bluetooth_local_name = bluetooth_local_name
    ) {
    firmata_word_output (
        pin_numbers = a5, 10:12
        value = word
        )
    firmata_digital_input (
        pin_number = 9
        value = bit0
        )
    firmata_digital_input (
        pin_number = a1
        value = bit1
        )
    firmata_digital_input (
        pin_number = a0
        value = bit2
        )
    firmata_digital_input (
        pin_number = 13
        value = bit3
        )
}


var index = 0

%define output_bits = [[0,0,0,0,0],
                       [1,0,0,0,0],
                       [0,1,0,0,0],
                       [0,0,1,0,0],
                       [0,0,0,1,0],
                       [1,1,1,1,0],
                       [0,0,0,0,1]]

%define update_word ()
    index += 1
    word = (output_bits[index][0] * 1 +
            output_bits[index][1] * 2 +
            output_bits[index][2] * 4 +
            output_bits[index][3] * 8 +
            output_bits[index][4] * 16)
%end

%define wait_and_assert ()
    wait (1s)
    assert (bit0 == output_bits[index][0])
    assert (bit1 == output_bits[index][1])
    assert (bit2 == output_bits[index][2])
    assert (bit3 == output_bits[index][3])
%end


protocol {
    index = -1
    update_word ()

    block (nsamples = 6) {
        bit0 = !(output_bits[index][0])
        bit1 = !(output_bits[index][1])
        bit2 = !(output_bits[index][2])
        bit3 = !(output_bits[index][3])

        report ('Starting device')
        start_io_device (firmata)

        wait_and_assert ()

        update_word ()

        wait_and_assert ()

        report ('Stopping device')
        stop_io_device (firmata)
    }
}

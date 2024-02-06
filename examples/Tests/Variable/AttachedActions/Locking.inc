%require scope


var num_ones_counted = 0
var num_twos_counted = 0

var value = 0 (scope = scope) {
    choose {
        when (value == 1) {
            num_ones_counted += 1
        }
        when (value == 2) {
            num_twos_counted += 1
        }
    }
}


var running = false
var num_ones_assigned = 0
var num_twos_assigned = 0
var num_repeats = 1000

protocol {
    num_ones_counted = 0
    num_twos_counted = 0

    running = false
    num_ones_assigned = 0
    num_twos_assigned = 0

    run_python_file ('count_events.py')
    on_exit {
        run_python_string ('unregister_event_callbacks()')
    }

    schedule (
        delay = 100ms
        duration = 0
        repeats = 1
        ) {
        running = true
        while (num_ones_assigned < num_repeats) {
            value = 1
            num_ones_assigned += 1
        }
        running = false
    }

    wait_for_condition (
        condition = running
        timeout = 200ms
        )
    while (num_twos_assigned < num_repeats) {
        value = 2
        num_twos_assigned += 1
    }

    wait_for_condition (
        condition = not running
        timeout = 1s
        )

    report ('num_ones_assigned = $num_ones_assigned')
    report ('num_twos_assigned = $num_twos_assigned')
    report ('num_ones_counted = $num_ones_counted')
    report ('num_twos_counted = $num_twos_counted')

    assert (num_ones_assigned == num_repeats)
    assert (num_twos_assigned == num_repeats)
    assert (num_ones_counted == num_repeats)
    assert (num_twos_counted == num_repeats)

    run_python_string ('check_event_counts()')
}

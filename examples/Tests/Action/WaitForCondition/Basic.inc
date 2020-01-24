var start_time = 0
var value = 123


%define perform_basic_tests ()
    start_time = now()
    wait_for_condition (
        condition = now() > start_time + 50ms
        timeout = 100ms
        timeout_message = 'Unexpected timeout'
        )
    assert (now() - (start_time + 50ms) < 1ms)

    start_time = now()
    wait_for_condition (
        condition = now() > start_time + 50ms
        timeout = 25ms
        stop_on_timeout = NO
        )
    assert (now() - (start_time + 25ms) < 1ms)

    start_time = now()
    wait_for_condition (
        condition = now() > start_time + 50ms
        timeout = 25ms
        timeout_message = 'The wait timed out; value = $value'
        stop_on_timeout = NO
        )
    assert (now() - (start_time + 25ms) < 1ms)

    start_time = now()
    wait_for_condition (
        condition = now() > start_time + 50ms
        timeout = 25ms
        )
%end

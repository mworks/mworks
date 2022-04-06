%define size = 2
%define offset = 5

var eye_x = 0
var eye_y = 0 {
    update_display ()
}

circle eye_marker (
    color = 0,0,0
    x_size = size / 2
    x_position = eye_x
    y_position = eye_y
    )

%define fixate ()
    eye_x = offset
    eye_y = offset
%end

%define defixate ()
    eye_x = 0
    eye_y = 0
%end

var eye_in_window = false {
    report ('eye_in_window = $eye_in_window')
}


%define assert_and_wait (expr)
    assert (expr)
    wait (1s)
%end

protocol {
    // Show eye marker
    live_queue_stimulus (eye_marker)
    update_display ()

    // Fixation point is not visible, so eye_in_window should not be set
    assert_and_wait (not eye_in_window)
    fixate ()
    assert_and_wait (not eye_in_window)
    defixate ()
    assert_and_wait (not eye_in_window)

    // Show fixation point
    queue_stimulus (fp)
    send_stimulus_to_back (fp)
    update_display ()

    // Fixation point is visible, so eye_in_window *should* be set
    assert_and_wait (not eye_in_window)
    fixate ()
    assert_and_wait (eye_in_window)
    defixate ()
    assert_and_wait (not eye_in_window)

    // Hide fixation point.  Doing so should cause eye_in_window to be cleared
    // if set.
    eye_in_window = true
    dequeue_stimulus (fp)
    update_display ()

    // Fixation point is once again not visible, so eye_in_window should no
    // longer be set
    assert_and_wait (not eye_in_window)
    fixate ()
    assert_and_wait (not eye_in_window)
    defixate ()
    assert_and_wait (not eye_in_window)
}

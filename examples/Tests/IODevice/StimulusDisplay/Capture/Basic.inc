var capture_format = ''
var capture_height_pixels = [300, 350, 400][disc_rand(0,2)]
var capture_enabled = false

stimulus_display (
    capture_format = capture_format
    capture_height_pixels = capture_height_pixels
    capture_enabled = capture_enabled
    )


%define size = 10

circle red_circle (
    color = 1,0,0
    x_size = size
    x_position = -size
    )

circle green_circle (
    color = 0,1,0
    x_size = size
    x_position = 0
    )

circle blue_circle (
    color = 0,0,1
    x_size = size
    x_position = size
    )


var display_bounds = {}

protocol {
    display_bounds = display_bounds()
    run_python_file ('check_capture.py')

    block (nsamples = 3) {
        queue_stimulus (red_circle)
        update_display ()
        wait (1s)
        dequeue_stimulus (red_circle)

        queue_stimulus (green_circle)
        update_display ()
        wait (1s)
        dequeue_stimulus (green_circle)

        queue_stimulus (blue_circle)
        update_display ()
        wait (1s)
        dequeue_stimulus (blue_circle)

        capture_enabled = !capture_enabled
    }

    run_python_string ('unregister_event_callbacks()')
    capture_enabled = false

    run_python_string ('check_captures()')
}

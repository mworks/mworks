//
// High effort test:  Capture every frame for 60 seconds.  Half the frame is
// full-color, random noise, updated every refresh, and therefore
// uncompressible.
//


stimulus_display (
    capture_format = capture_format
    capture_height_pixels = capture_height_pixels
    redraw_on_every_refresh = true
    )


white_noise_background dynamic_noise (
    grayscale = false
    randomize_on_draw = true
    )

rectangle static_rect (
    color = 0.5, 0.5, 0.5
    x_size = display_bounds('right') - display_bounds('left')
    y_size = display_bounds('top') - display_bounds('bottom')
    y_position = display_bounds('bottom')
    )


protocol {
    queue_stimulus (dynamic_noise)
    queue_stimulus (static_rect)
    update_display ()
    wait (60s)
}

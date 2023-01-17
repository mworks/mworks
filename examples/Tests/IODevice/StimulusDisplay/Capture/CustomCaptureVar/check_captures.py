standard_captures = []
custom_captures = []


def check_captures():
    assert len(standard_captures) == 0
    assert len(custom_captures) == 3

    previous_time = 0

    for c in custom_captures:
        assert c.time > 0
        assert c.time > previous_time
        previous_time = c.time

        data = c.data
        assert isinstance(data, bytes)
        assert len(data) > 0

    # Clear all events
    standard_captures[:] = []
    custom_captures[:] = []


register_event_callback('#stimDisplayCapture', standard_captures.append)
register_event_callback('display_capture', custom_captures.append)

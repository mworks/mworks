import struct


captures = []
updates = []


def check_png(data):
    # Determine expected width and height
    display_bounds = getvar('display_bounds')
    display_width = display_bounds['right'] - display_bounds['left']
    display_height = display_bounds['top'] - display_bounds['bottom']
    expected_height = getvar('capture_height_pixels')
    expected_width = int(expected_height * display_width / display_height)

    # PNG header
    assert data.startswith(b'\x89PNG\x0D\x0A\x1A\x0A')

    # IHDR chunk
    ihdr_index = data.find(b'IHDR')
    assert ihdr_index == 12
    ihdr_data = struct.unpack('>IIBBBBB',
                              data[ihdr_index+4:ihdr_index+17])
    assert expected_width - 1 <= ihdr_data[0] <= expected_width + 1
    assert ihdr_data[1] == expected_height
    assert ihdr_data[2:] == (8, 6, 0, 0, 0)  # 8 bit, RGBA, etc.

    # IDAT chunk
    idat_index = data.find(b'IDAT')
    assert idat_index > ihdr_index
    idat_length = struct.unpack('>I', data[idat_index-4:idat_index])[0]
    assert 0 < idat_length < len(data)

    # IEND chunk
    assert data.endswith(b'\x00\x00\x00\x00IEND\xaeB`\x82')


def check_jpeg(data):
    # JPEG is more complicated to parse than PNG.  Just do minimal checks.

    # SOI segment
    assert data.startswith(b'\xFF\xD8')

    # EOI segment
    assert data.endswith(b'\xFF\xD9')


def check_captures():
    assert len(captures) == 3
    assert len(updates) == 9

    previous_time = 0

    check_format = {
        'PNG': check_png,
        'JPEG': check_jpeg,
        }[getvar('capture_format')]

    for c, u in zip(captures, updates[3:6]):
        assert c.time > 0
        assert c.time == u.time
        assert c.time > previous_time
        previous_time = c.time

        data = c.data
        assert isinstance(data, bytes)
        assert len(data) > 0
        check_format(data)

    # Clear all events
    captures[:] = []
    updates[:] = []


register_event_callback('#stimDisplayCapture', captures.append)
register_event_callback('#stimDisplayUpdate', updates.append)

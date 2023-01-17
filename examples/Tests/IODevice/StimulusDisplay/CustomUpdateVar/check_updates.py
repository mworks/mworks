standard_updates = []
custom_updates = []


def check_updates():
    assert len(standard_updates) == 0
    assert len(custom_updates) == 9

    previous_time = 0
    stim_names = ('red_circle', 'green_circle', 'blue_circle')

    for i, u in enumerate(custom_updates):
        assert u.time > 0
        assert u.time > previous_time
        previous_time = u.time

        data = u.data
        assert isinstance(data, list)
        assert len(data) == 1

        data = data[0]
        assert isinstance(data, dict)
        assert data['name'] == stim_names[i % 3]

    # Clear all events
    standard_updates[:] = []
    custom_updates[:] = []


register_event_callback('#stimDisplayUpdate', standard_updates.append)
register_event_callback('display_update', custom_updates.append)

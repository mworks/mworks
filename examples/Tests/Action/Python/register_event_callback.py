from collections import deque


reverse_codec = get_reverse_codec()


all_expected = deque([('foo', 1), ('bar', 2), ('foo', 3), ('bar', 4)])
foo_expected = deque([('foo', 1), ('foo', 3)])
bar_expected = deque([('bar', 2), ('bar', 4)])

last_event_time = 0


def check_event(evt, expected_events):
    global last_event_time
    expected = expected_events.popleft()
    assert isinstance(evt, Event)
    assert evt.code == reverse_codec[expected[0]]
    assert evt.time > 0 and evt.time >= last_event_time
    assert evt.data == expected[1]
    last_event_time = evt.time


def all_events(evt):
    if evt.code not in (reverse_codec['foo'], reverse_codec['bar']):
        return
    check_event(evt, all_expected)
    if evt.data == 3:
        raise RuntimeError('This is a simulated error')


def foo_by_name(evt):
   check_event(evt, foo_expected)


def bar_by_code(evt):
   check_event(evt, bar_expected)


register_event_callback(all_events)
register_event_callback('foo', foo_by_name)
register_event_callback(reverse_codec['bar'], bar_by_code)

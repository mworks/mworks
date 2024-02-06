import time


num_one_events = 0
num_two_events = 0


def count_event(evt):
    global num_one_events, num_two_events

    if evt.data == 1:
        num_one_events += 1
    elif evt.data == 2:
        num_two_events += 1


def check_event_counts():
    num_repeats = getvar('num_repeats')

    while num_one_events + num_two_events < 2 * num_repeats:
        time.sleep(0.1)

    if num_one_events != num_repeats:
        error('num_one_events = %d' % num_one_events)
    if num_two_events != num_repeats:
        error('num_two_events = %d' % num_two_events)


register_event_callback('value', count_event)

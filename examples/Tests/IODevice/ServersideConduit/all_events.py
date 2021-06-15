import queue

from shared import resource_name

from mworks.conduit import IPCClientConduit
from mworks.data import ReservedEventCode


def main():
    with IPCClientConduit(resource_name) as conduit:
        rc = conduit.reverse_codec

        # Ignore reserved event codes and system variables
        ignored_codes = tuple(ReservedEventCode)
        ignored_codes += tuple(code for name, code in rc.items()
                               if name.startswith('#'))

        events = queue.Queue()
        num_ignored = 0

        def cb(evt):
            nonlocal num_ignored
            if evt.code in ignored_codes:
                num_ignored += 1
            else:
                events.put(evt)

        conduit.register_callback_for_all_events(cb)

        def assert_event(expected_name, expected_data):
            evt = events.get(timeout=5)
            assert rc[expected_name] == evt.code
            assert expected_data == evt.data

        assert_event('foo', 1)
        assert_event('bar', 'one')
        assert_event('foo', 2)
        assert_event('bar', 'two')
        assert_event('foo', 3)
        assert_event('bar', 'three')

        # If we're really receiving all events, there should be many that we
        # ignored
        assert num_ignored > 0


if __name__ == '__main__':
    main()

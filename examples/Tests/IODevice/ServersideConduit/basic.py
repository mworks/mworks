import queue

from shared import resource_name

from mworks.conduit import IPCClientConduit


def main():
    with IPCClientConduit(resource_name) as conduit:
        rc = conduit.reverse_codec
        events = queue.Queue()

        def callback(name):
            def cb(evt):
                events.put((rc[name], evt))
            return cb

        conduit.register_callback_for_code(rc['foo'], callback('foo'))
        conduit.register_callback_for_name('bar', callback('bar'))

        def assert_event(expected_data):
            expected_code, evt = events.get(timeout=5)
            assert expected_code == evt.code
            assert expected_data == evt.data

        assert_event(1)
        assert_event('one')
        assert_event(2)
        assert_event('two')
        assert_event(3)
        assert_event('three')


if __name__ == '__main__':
    main()

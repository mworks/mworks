import queue
import time

from shared import resource_name

from mworks.conduit import IPCClientConduit


num_vars = 1000
num_assignments = 1000


def main():
    with IPCClientConduit(resource_name) as conduit:
        rc = conduit.reverse_codec
        events = queue.Queue()

        # Wait until the server is done announcing the initial values of all
        # variables, since we don't want to receive those
        time.sleep(2)

        def callback(name):
            def cb(evt):
                events.put((rc[name], evt))
            return cb

        for index in range(num_vars):
            name = 'var%03d' % index
            conduit.register_callback_for_name(name, callback(name))

        def assert_event(expected_data):
            expected_code, evt = events.get(timeout=5)
            assert expected_code == evt.code
            assert expected_data == evt.data

        conduit.send_data(rc['client_ready'], True)

        for index in range(num_assignments):
            for _ in range(num_vars):
                assert_event(index + 1)

        conduit.send_data(rc['client_ready'], True)


if __name__ == '__main__':
    main()

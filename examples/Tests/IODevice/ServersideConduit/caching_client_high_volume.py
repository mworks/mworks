import time

from shared import resource_name

from mworks.conduit import CachingIPCClientConduit


num_vars = 1000
num_assignments = 1000


def main():
    with CachingIPCClientConduit(resource_name) as conduit:
        rc = conduit.reverse_codec

        # Wait until the server is done announcing the initial values of all
        # variables
        time.sleep(2)

        last_var_name = 'var%03d' % (num_vars - 1)
        assert last_var_name in conduit
        assert 0 == conduit[last_var_name]

        conduit.send_data(rc['client_ready'], True)

        # Time out after 30 seconds
        timeout_time = time.time() + 30

        while conduit[last_var_name] != num_assignments:
            assert time.time() < timeout_time
            time.sleep(1)

        conduit.send_data(rc['client_ready'], True)


if __name__ == '__main__':
    main()

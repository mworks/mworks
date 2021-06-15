import time

from shared import resource_name

from mworks.conduit import CachingIPCClientConduit


def main():
    with CachingIPCClientConduit(resource_name) as conduit:
        rc = conduit.reverse_codec

        def assert_values(foo_value, bar_value):
            # Pause while the server sends values and the conduit caches them
            time.sleep(1)

            # Verify that the expected values have been received and cached
            assert rc['foo'] in conduit
            assert foo_value == conduit[rc['foo']]
            assert 'bar' in conduit
            assert bar_value == conduit['bar']

            # Let the server know we're ready for the next values
            conduit.send_data(rc['client_ready'], True)

        assert_values(0, 'zero')
        assert_values(1, 'one')
        assert_values(2, 'two')
        assert_values(3, 'three')

        # Non-existent names and codes should not be present
        assert 'foobar' not in conduit
        assert 123 not in conduit


if __name__ == '__main__':
    main()

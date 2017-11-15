from ._mworks import (_IPCClientConduit,
                      _IPCServerConduit,
                      _IPCAccumulatingClientConduit,
                      _IPCAccumulatingServerConduit)


class IPCClientConduit(_IPCClientConduit):
    pass


class IPCServerConduit(_IPCServerConduit):
    pass


class IPCAccumClientConduit(_IPCAccumulatingClientConduit):
    pass


class IPCAccumServerConduit(_IPCAccumulatingServerConduit):
    pass

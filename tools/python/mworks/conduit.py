from _mworks import (_IPCClientConduit,
                     _IPCServerConduit,
                     _IPCAccumulatingClientConduit,
                     _IPCAccumulatingServerConduit)

try:
    import numpy
    _bool_type = (bool, numpy.bool_)
    _int_type = (int, numpy.signedinteger)
    _float_type = (float, numpy.floating)
except ImportError:
    _bool_type = bool
    _int_type = int
    _float_type = float


class _ConduitMixin(object):

    def send_data(self, code, data):
        if isinstance(data, _bool_type):
            self.send_object(code, bool(data))
        elif isinstance(data, _int_type):
            self.send_integer(code, data)
        elif isinstance(data, _float_type):
            self.send_float(code, data)
        else:
            self.send_object(code, data)


class IPCClientConduit(_ConduitMixin, _IPCClientConduit):
    pass


class IPCServerConduit(_ConduitMixin, _IPCServerConduit):
    pass


class IPCAccumClientConduit(_ConduitMixin, _IPCAccumulatingClientConduit):
    pass


class IPCAccumServerConduit(_ConduitMixin, _IPCAccumulatingServerConduit):
    pass

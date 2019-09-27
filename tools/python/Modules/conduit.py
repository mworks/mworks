from . import _mworks


class _IPCConduit(_mworks._IPCConduit):

    def __init__(self, resource_name, correct_incoming_timestamps=False):
        super().__init__(resource_name,
                         correct_incoming_timestamps,
                         self._event_transport_type)

    # The following three methods aren't needed but remain for backwards
    # compatibility

    def send_float(self, code, value):
        self.send_data(code, value)

    def send_integer(self, code, value):
        self.send_data(code, value)

    def send_object(self, code, value):
        self.send_data(code, value)


class IPCClientConduit(_IPCConduit):

    _event_transport_type = _mworks.client_event_transport


class IPCServerConduit(_IPCConduit):

    _event_transport_type = _mworks.server_event_transport


class _IPCAccumConduit(_IPCConduit):

    def __init__(self, resource_name, start_event, end_event, events_to_watch,
                correct_incoming_timestamps=False):
        super().__init__(resource_name, correct_incoming_timestamps)
        self.start_event = start_event
        self.end_event = end_event
        self.events_to_watch = events_to_watch

    def initialize(self):
        ok = super().initialize()
        if ok:
            self._accumulating = False
            self._event_buffer = []
            self._bundle_callback = None
            self.register_callback_for_name(self.start_event,
                                            self._start_accumulating)
            self.register_callback_for_name(self.end_event,
                                            self._stop_accumulating)
            for event in self.events_to_watch:
                self.register_callback_for_name(event, self._accumulate)
        return ok

    def register_bundle_callback(self, callback):
        self._bundle_callback = callback

    def _start_accumulating(self, evt):
        self._accumulating = True

    def _accumulate(self, evt):
        if self._accumulating:
            self._event_buffer.append(evt)

    def _stop_accumulating(self, evt):
        self._accumulating = False
        if self._bundle_callback:
            self._bundle_callback(self._event_buffer)
        self._event_buffer = []


class IPCAccumClientConduit(_IPCAccumConduit):

    _event_transport_type = _mworks.client_event_transport


class IPCAccumServerConduit(_IPCAccumConduit):

    _event_transport_type = _mworks.server_event_transport

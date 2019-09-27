import _mworkscore
from _mworkscore import *


def register_event_callback(name_or_code, callback=None):
    if callback is None:
        _mworkscore._register_event_callback(name_or_code)
    elif isinstance(name_or_code, str):
        _mworkscore._register_event_callback_for_name(name_or_code, callback)
    else:
        _mworkscore._register_event_callback_for_code(int(name_or_code),
                                                      callback)

from _conduit import *

def send_data(self, code, data):
    dtype = data.__class__
    if dtype == float or dtype == numpy.float32 or dtype == numpy.float64:
        self.send_float(code, data)
    elif dtype == int or dtype == numpy.int16 or dtype == numpy.int32 or dtype == numpy.int64:
        self.send_integer(code, data)
    else:
        self.send_object(code, data)


IPCClientConduit.send_data = send_data
IPCServerConduit.send_data = send_data


def handle_codec_(self, evt):
    self.reverse_codec = {}
    self.codec = {}

    codec_datum = evt.value()
    
    for k in codec_datum.keys():
        name = codec_datum[k]["tagname"]
        self.codec[k] = name 
        self.reverse_codec[name] = k
   
def initialize_(self):
    # register a pure python handler for the codec
    #self.register_callback_for_code(0, lambda e: self.handle_codec(self, e))
    
    # call the C++ initialize method
    self.__initialize()
    
IPCClientConduit.initialize = initialize_
IPCServerConduit.initialize = initialize_

#IPCClientConduit.handle_codec = handle_codec_
#IPCServerConduit.handle_codec = handle_codec_

def __get_converted_datum(self):
    return self.__convert_mw_datum_to_python(self.mw_datum)

Event.data = property(__get_converted_datum)


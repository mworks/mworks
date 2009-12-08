#
#  monkeyworks/data.py
#  DataFileIndexer
#
#  Created by David Cox on 8/17/09.
#  Copyright (c) 2009 Harvard University. All rights reserved.
#

from _data import *
import numpy

# Event add-ons

# we need to do a little bit of sleight of hand to 
# python-ify the scarab datum contained within an event
# This is all transparent to the user
#def get_value(self):
#    return __extract_event_value(self)

#Event.value = property(get_value)

class FileNotLoadedException(Exception):
    pass
    
class NoValidCodecException(Exception):
    pass
    

# MWKFile add-ons

def get_events(self, **kwargs):

    event_codes = []
    time_range = []

    # shortcut to argument-free version
    if "codes" not in kwargs and "time_range" not in kwargs:
        return self.__fetch_all_events()

    codec = self.codec
    
    if codec is None:
        raise NoValidCodecException
        
    reverse_codec = self.reverse_codec
     
    if reverse_codec is None:
        raise NoValidCodecException

    if "codes" in kwargs:
        event_codes = kwargs["codes"]
        
        for i in range(0, len(event_codes)):
            code = event_codes[i]
            if(type(code) == str):
                if(code in reverse_codec):
                    event_codes[i] = reverse_codec[code]
        
    else:
        event_codes = codec.keys()  # all events
    
    if "time_range" in kwargs:
        time_range = kwargs["time_range"]
    else:
        time_range = [self.minimum_time, self.maximum_time]

    # TODO: convert possible string-based event codes

    events = self.__fetch_events(event_codes, time_range[0], time_range[1])
    
    return events

MWKFile.get_events = get_events

def get_codec(self):

    if not self.loaded:
        raise FileNotLoadedException

    e = self.__fetch_events([0])
    if(len(e) == 0):
        return None
    
    raw_codec = e[0].value
    
    codec = {}
    for key in raw_codec.keys():
        codec[key] = raw_codec[key]["tagname"]
    return codec
    
MWKFile.codec = property(get_codec)


def get_reverse_codec(self):

    if not self.loaded:
        raise FileNotLoadedException

    c = self.codec
    keys = c.keys()
    values = c.values()
    rc = {}
    for i in range(0, len(keys)):
        k = keys[i]
        v = values[i]
        print("key: %d, value %s" % (k,v))
        rc[v] = k
    return rc

MWKFile.reverse_codec = property(get_reverse_codec)
   
    
def read_event(self):
    result = self.__read_event()
    if(result.empty):
        result = None
    return result
    
MWKStream.read_event = read_event
    
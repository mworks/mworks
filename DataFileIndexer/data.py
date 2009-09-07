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



# MWKFile add-ons

def get_events(self, **kwargs):

    event_codes = []
    time_range = []

    if "codes" in kwargs:
        event_codes = kwargs["codes"]
    else:
        event_codes = []
    
    if "time_range" in kwargs:
        time_range = kwargs["time_range"]
    else:
        time_range = [-numpy.inf, numpy.inf]

    # TODO: convert possible string-based event codes

    events = self.__fetch_events(event_codes, time_range[0], time_range[1])
    
    return events

MWKFile.events = property(get_events)
/*
 *  EventWrapper.cpp
 *  DataFileIndexer
 *
 *  Created by bkennedy on 2/19/08.
 *  Copyright 2008 MIT. All rights reserved.
 *
 */

#include "EventWrapper.h"

EventWrapper::EventWrapper(ScarabDatum *new_datum) {
	datum = scarab_copy_datum(new_datum);
}

EventWrapper::EventWrapper(const EventWrapper &sdw) {
	datum = scarab_copy_datum(sdw.getDatum());
}


EventWrapper::~EventWrapper() {
    if(!empty()){
        scarab_free_datum(datum);
    }
}

ScarabDatum *EventWrapper::getDatum() const {
	return datum;
}




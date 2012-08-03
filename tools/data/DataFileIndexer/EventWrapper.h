/*
 *  EventWrapper.h
 *  DataFileIndexer
 *
 *  Created by bkennedy on 2/19/08.
 *  Copyright 2008 MIT. All rights reserved.
 *
 */

#ifndef SCARAB_DATUM_WRAPPER_H
#define SCARAB_DATUM_WRAPPER_H

#include "Scarab/scarab.h"
#include <MWorksCore/EventConstants.h>
#include <MWorksCore/Utilities.h>
using namespace mw;

class EventWrapper
	{
	protected:
		ScarabDatum *datum;		
	public:
		EventWrapper(ScarabDatum *);
		EventWrapper(const EventWrapper &sdw);
		~EventWrapper();
		ScarabDatum *getDatum() const;
        
        bool empty(){
            return (datum == NULL);
        }
        
        int getEventCode(){
            if(empty()){
                return -1;
            }
            ScarabDatum *code_datum = scarab_list_get(datum, SCARAB_EVENT_CODEC_CODE_INDEX);
            return code_datum->data.integer;
        }
        
        MWorksTime getTime(){
            if(empty()){
                return 0LL;
            }
            ScarabDatum *time_datum = scarab_list_get(datum, SCARAB_EVENT_TIME_INDEX);
            return time_datum->data.integer;
        }
        
        ScarabDatum *getPayload(){
            if(empty() || (datum->data.list->size < SCARAB_PAYLOAD_EVENT_N_TOPLEVEL_ELEMENTS)){
                return NULL;
            }
            ScarabDatum *payload_datum = scarab_list_get(datum, SCARAB_EVENT_PAYLOAD_INDEX);
            return payload_datum;    
        }
        
        // This is needed for Python bindings
        bool operator==(EventWrapper const& event) const { return datum == event.datum; }
	};


#endif //SCARAB_DATUM_WRAPPER_H

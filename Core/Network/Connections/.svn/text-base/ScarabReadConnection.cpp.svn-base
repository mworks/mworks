/**
 * ScarabReadConnection.cpp
 *
 * History:
 * Paul Jankunas on 12/1/04 - Created.
 * Paul Jakunas on 1/28/05 - Modifying the service routine to work with
 *                          codec.
 * Paul Jankunas on 04/26/05 - Changed the way codec messages are sent.
 *                      so now have to update how they are received.
 *
 * Copyright 2004 MIT. All rights reserved.
 */

#include "ScarabReadConnection.h"
#include "VariableRegistry.h"
#include "Scarab/scarab_utilities.h"
#include "ScarabServices.h"
#include "EventBuffer.h"
#include "boost/bind.hpp"

#include "Event.h"
using namespace mw;

// this calls the service function. static so no one outside this file can
// see it.
static void * read(const shared_ptr<ScarabConnection> &sc);

ScarabReadConnection::ScarabReadConnection(shared_ptr<BufferManager> _buffer_manager, std::string uri) :
                                                    ScarabConnection(_buffer_manager, uri) { }

void ScarabReadConnection::startThread(int interval) {
    if(thread) { return; } // already running
	mdebug("Starting read thread (interval = %d)", interval);
	
	shared_ptr <ScarabConnection> this_one = shared_from_this();
	
	shared_ptr<Scheduler> scheduler = Scheduler::instance();	
    thread = scheduler->scheduleUS(std::string(FILELINE) + ": Scarab Read Thread: " + uri,
								   0,
								   interval, 
								   M_REPEAT_INDEFINITELY,
								   boost::bind(read, this_one),
								   M_DEFAULT_NETWORK_PRIORITY,
								   0,  // No warnings
								   //M_DEFAULT_NETWORK_WARN_SLOP_US,
								   M_DEFAULT_NETWORK_FAIL_SLOP_US,
								   M_MISSED_EXECUTION_DROP);
}

static void * read(const shared_ptr<ScarabConnection> &src) {
    //mdebug("Called read");
    int rc = src->service();
    // check return code
    if(rc <= 0) {
        mdebug("Stopping Thread read with id %d",
			   src->getID());
        src->stopThread();
        src->disconnect();
    }
    return NULL;
}

Event * ScarabReadConnection::generateEventFromList(ScarabDatum * listDatum) {
    return NULL;
}

int ScarabReadConnection::service() {
	boost::mutex::scoped_lock lock(connectLock);
	if(servicing) {
        // we are already servicing this connection.
        mdebug("Already Servicing! Reading");
        return 1;
    }

    servicing = true;
    //mdebug("Calling read function on id %d", cid);
    if(interrupt) {
        mwarning(M_NETWORK_MESSAGE_DOMAIN,
					"Interrupt set for read connection id %d", cid);
        servicing = false;
        term = true;
        if(sibling) {
            sibling->setInterrupt(true);
        }
        return 0;
    }
    
    if(getScarabError(pipe)) {
        // TODO: should this be a warning or debug message
		mwarning(M_NETWORK_MESSAGE_DOMAIN, "Session Failure on ID %d", cid);
        servicing = false;
        if(sibling) {
            sibling->setInterrupt(true);
        }
        term = true;
        return -1;
    }

    ScarabDatum *datum;
    
	
	while(datum = scarab_read(pipe)) {
    
		// DDC added: experimental attempt to stop intermittant crashes in the read thread of the
		// client under bursty network load
		if(getScarabError(pipe)) {
			// TODO: should this be a warning or debug message
			mwarning(M_NETWORK_MESSAGE_DOMAIN, "Session Failure on ID %d", cid);
			servicing = false;
			if(sibling) {
				sibling->setInterrupt(true);
			}
			term = true;
			return -1;
		}
			
				    
		// the connection stats is total shit
        // connectionStats->eventReceived((EventCode)datum->type, sizeof(datum));
        
		switch(datum->type) {
            case 0://SCARAB_NULL
            case 1://SCARAB_INTEGER
            case 2://SCARAB_FLOAT
            case 3://SCARAB_FLOAT_INF
            case 4://SCARAB_FLOAT+NAN
				mwarning(M_NETWORK_MESSAGE_DOMAIN,
						"Bare numerical scarab datum in the network stream");
                break;
            case 6://SCARAB_LIST
				
				ScarabDatum *code = scarab_list_get(datum, 
								SCARAB_EVENT_CODEC_CODE_INDEX);
				if(code->type == SCARAB_INTEGER && code->data.integer ==
									M_TERMINATION_EVENT_CODE) { 
					//termination event
                    mwarning(M_NETWORK_MESSAGE_DOMAIN, 
							"Received Termination code on id %d", cid);
                    servicing = false;
                    setInterrupt(true);
                    if(sibling) {
                        sibling->setInterrupt(true);
                    }
                    term = true;
                    return 0;
				}
				
				if(code->type == SCARAB_LIST){
					// "Macro" event (a list of events, sent as one TCP
					// packet for the sake of efficiency)
					/*fprintf(stderr, "Reading macroevent with %d events\n",
									datum->data.list->size);
					fflush(stderr);*/
					
					for(int i = 0; i < datum->data.list->size; i++){
						ScarabDatum *subevent = scarab_list_get(datum, i);
						
						shared_ptr<Event> event = shared_ptr<Event>(new Event(subevent));					
						
						
						if(event == NULL){
							// TODO: warn
							mwarning(M_NETWORK_MESSAGE_DOMAIN, 
										"NULL Event from read stream!");
							servicing = false;
							
							scarab_free_datum(datum);
							return 1;
							//break;
						}
						
						buffer_manager->putIncomingNetworkEvent(event);
					}
				
				} else {
				
					shared_ptr<Event> event = shared_ptr<Event>(new Event(datum));					
					
					if(event == NULL){
						// TODO: warn
						mwarning(M_NETWORK_MESSAGE_DOMAIN, 
									"NULL Event from read stream!");
						servicing = false;
						scarab_free_datum(datum);
						return 1;
						//break;
					}
					
					buffer_manager->putIncomingNetworkEvent(event);
					
				}
				
				break;
            case 7://SCARAB_OPAQUE
				mwarning(M_NETWORK_MESSAGE_DOMAIN,
							"Bare SCARAB_OPAQUE datum in the network stream");
                break;
            case 5://SCARAB_DICT
				mwarning(M_NETWORK_MESSAGE_DOMAIN,
							"Bare SCARAB_DICT datum in the network stream");
                break;
            default:
                mwarning(M_NETWORK_MESSAGE_DOMAIN,
					"Bad Data Type Received From Scarab Stream!");
                break;
        }
		
		// DDC: fixes leak
		scarab_free_datum(datum);

    }
	
    if(scarab_session_read_should_die(pipe)) {
        servicing = false;
        setInterrupt(true);
        if(sibling) {
            sibling->setInterrupt(true);
        }
        term = true;
        
        return 0;
    }
    //mdebug("service read going false");
    servicing = false;
    return 1;
}
 

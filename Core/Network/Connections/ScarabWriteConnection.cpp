/**
 * ScarabWriteConnection.cpp
 *
 * History:
 * Paul Jankunas on 12/1/04 - Created.
 * Paul Jankunas on 04/26/05 - Modified the way events are put on the network
 *              to make access more uniform.
 * Paul Jankunas on 06/15/05 - Adding data type to generic event list.
 *
 * Copyright 2004 MIT. All rights reserved.
 */

#include "ScarabWriteConnection.h"
#include "VariableRegistry.h"
#include "ScarabServices.h"
#include "boost/bind.hpp"
#include "Event.h"
using namespace mw;

#define min(a,b) (((a)<(b))?(a):(b))

// this calls the service function. static so no one outside this file can
// see it.
static void * write(const shared_ptr<ScarabConnection> &sc);

ScarabWriteConnection::ScarabWriteConnection(shared_ptr<BufferManager> _buffer_manager, std::string uri) :
                                                    ScarabConnection(_buffer_manager, uri) {
    //buffer_reader = buffer_manager->getNewDisplayBufferReader();
}
                    
void ScarabWriteConnection::startThread(int interval) {
    if(thread) { return; } // already running
	
	// DDC: moved here.  Don't create buffer reader until it is ready to move
	buffer_reader = buffer_manager->getNewDisplayBufferReader();
	
	shared_ptr<ScarabConnection> this_one = shared_from_this();
	
	shared_ptr<Scheduler> scheduler = Scheduler::instance();	
    thread = scheduler->scheduleUS(std::string(FILELINE) + ": Scarab Write Thread: " + uri,
								   0,
								   interval, 
								   M_REPEAT_INDEFINITELY,
								   boost::bind(write, this_one),
								   M_DEFAULT_NETWORK_PRIORITY,
								   0, // No warnings
								   //M_DEFAULT_NETWORK_WARN_SLOP_US,
								   M_DEFAULT_NETWORK_FAIL_SLOP_US,
								   M_MISSED_EXECUTION_DROP);
}

void ScarabWriteConnection::stopThread() {
	boost::mutex::scoped_lock lock(connectLock);
	buffer_reader = shared_ptr<EventBufferReader>();
    if(thread) {
        thread->cancel();
    }
}

static void * write(const shared_ptr<ScarabConnection> &swc) {
    int rc = swc->service();
    // check return code
    if(rc <= 0) {
        // an error has occurred on the socket
        // prob will want a kill thread option here
        // to stop things from blocking
        mdebug("Stopping Thread write with id %d",
			   swc->getID());
        swc->stopThread();
        swc->disconnect();
    }
    return NULL;
}

int ScarabWriteConnection::service() {
	boost::mutex::scoped_lock lock(connectLock);

	static int n_written = 0;
	
	shared_ptr <Clock> clock = Clock::instance();
	static MonkeyWorksTime start_time = clock->getCurrentTimeUS();
//    static int functionCount = 0;
//    mdebug("entering write service");
//    struct timespec time_to_sleep;
//    time_to_sleep.tv_sec = 0;
//    time_to_sleep.tv_nsec = 200000000; //200 ms
//    while(1) {
//        mdebug("start write while loop");
//        if(servicing) {
//            // we are already servicing this connection.
//            mdebug("Already Servicing Writing");
//            M_ISUNLOCK;
//            continue;
//            //1return 1;
//        }
        servicing = true;
    
    //    mdebug("Started servicing write function count = %d", functionCount);
//        functionCount++;
        if(getScarabError(pipe)) {
            mwarning(M_NETWORK_MESSAGE_DOMAIN, "Session Failure on ID %d", cid);
            servicing = false;
            if(sibling) {
                sibling->setInterrupt(true);
            }
            term = true;
            
            return -1;
        }

        // if we have been requested to get interrupted
        // if we are interrupted while processing events there will be a
        // slight delay while out task is re-scheduled
        if(interrupt) {
            mwarning(M_NETWORK_MESSAGE_DOMAIN,
					"Write Service Routine Interrupted on id %d", cid);
            if(sibling) {
                sibling->setInterrupt(true);
            }
            ScarabDatum * termEvent;
            // puts a control event of type termination into a scarab package
            // and sends the package
            termEvent = scarab_list_new(2);
			
			ScarabDatum *termCode = scarab_new_integer(M_TERMINATION_EVENT_CODE);
            scarab_list_put(termEvent, 0, termCode);
			scarab_free_datum(termCode);
			
			shared_ptr <Clock> clock = Clock::instance();
			ScarabDatum *time = scarab_new_integer(clock->getSystemTimeMS());
            scarab_list_put(termEvent, 1, time);
			scarab_free_datum(time);

            if(scarab_write(pipe, termEvent) == 0) {
                // success
                mdebug("Wrote termination message from id %d", cid);
                servicing = false;
                term = true; // mark the connection for termination
                scarab_free_datum(termEvent);
				return 0;
            } else {
                mwarning(M_NETWORK_MESSAGE_DOMAIN,
					"Failed to Write Termination Sequence on socket %d", cid);
                // even though there was an error we are going to 
                // terminate
                term = true; // mark the connection for termination
                scarab_free_datum(termEvent);
				return -1;
            }
			
			scarab_free_datum(termEvent);
        }
    
        // while we have events to process but havent been requested for
        // interruption.
        
		while(buffer_reader->nextEventExists() && !interrupt) {
            
            //MonkeyWorksTime event_time = newevent->getTime();
            //mEventType type = newevent->getEventType();
            //mVariable *var = newevent->getParam();
            ScarabDatum * scarab_event;
        			
			#define USE_EXPLICIT_BUFFERING	1
			#define BUFFER_HIGH_WATER_MARK	1
			#define MAX_EVENTS_TO_BUFFER	10000
			
			if(USE_EXPLICIT_BUFFERING && buffer_reader->hasAtLeastNEvents(BUFFER_HIGH_WATER_MARK)) {
			
				scarab_force_buffering(pipe, 1);
				
				// fill up the macro event
				int numEventsBuffered = 0;
				bool buffering = true;
				do {
					
					shared_ptr<Event> newevent = buffer_reader->getNextEvent();
					if(newevent == NULL) {
						// don't send anything if an event was NULL.
						servicing = false;
						scarab_free_datum(scarab_event);
						return 1;
					}
					
					scarab_event = newevent->toScarabDatum();

					// stop buffering when you run out of events, or you hit the max events
					if(!(buffering = buffer_reader->nextEventExists() && numEventsBuffered<MAX_EVENTS_TO_BUFFER)) {
						scarab_force_buffering(pipe, 0);
					}
					
					if(scarab_write(pipe, scarab_event) == 0) {
						n_written++;
					} else {
						merror(M_SYSTEM_MESSAGE_DOMAIN, "scarab buffered write error");
						servicing = false;
						return -1;
					}
					
					scarab_free_datum(scarab_event);
					++numEventsBuffered;
				} while(buffering);
				
			
			} else {
				
				// Single event write
				shared_ptr<Event> newevent = buffer_reader->getNextEvent();
				if(newevent == NULL) {
					// don't send anything if an event was NULL.
					servicing = false;
					return 1;
				}
				
				scarab_event = newevent->toScarabDatum();
								
				if(scarab_write(pipe, scarab_event) == 0) {
					n_written++;
				} else {
					merror(M_SYSTEM_MESSAGE_DOMAIN, "scarab write error");
					servicing = false;
					return -1;
				}
				
				scarab_free_datum(scarab_event);
				
			}
			
			// Some reporting
			#define  N  1000
			static int last_nwritten = 0;
			if(n_written - last_nwritten >= N){
				last_nwritten = n_written;
				
				MonkeyWorksTime now = clock->getCurrentTimeUS();
				
				/*fprintf(stderr, "last: %lld, now: %lld\n",
							start_time,
							now
							); */
//				fprintf(stderr, "%g events / sec (%d event in %lld usec)\n",
//							(double)N / howlong,
//							N, now - start_time
//							);
//				fprintf(stderr, "Number of items unserviced: %d\n",
//						buffer_reader->getNItemsUnserviced());
//				fflush(stderr);
				start_time = now;
			}
        } 


        servicing = false;

        return 1;
//    }
}
 

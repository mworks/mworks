/**
 * Event.h
 *
 * Description: This file contains the Events class and all its subclasses.
 * Event objects are used to send data to other parts of the program either
 * locally or remotely.  They can be accessed using an EventBufferReader
 * object.  Using the buffer reader events can be read in a FIFO style.  After
 * an event is read you must call advance() on the buffer reader to get
 * the next event enqueue. MORE TO COME
 *
 * Copyright (c) 2005 MIT. All rights reserved.
 */

#ifndef MONKEYWORKSCORE_EVENT_H_
#define MONKEYWORKSCORE_EVENT_H_


#include "ScarabServices.h"
#include "MonkeyWorksTypes.h"
#include "GenericData.h"
#include "EventConstants.h"
#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>

#include <boost/serialization/list.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/version.hpp>

/**
 *	----------------------
 *	Scarab Event Structure
 *  ---------------------- 
 *
 * All events have the following top-level list structure:
 *
 *		   0				   1					2
 * |  codec code   |   time (64bit,signed)  |	  payload      |
 *
 */
namespace mw {
	using namespace boost;
	
	
	class Event {
    protected:
		
		// what is this event's codec code
		int code;
		
		// when did this event occur
		MonkeyWorksTime time;
		
		Data data;
		shared_ptr<Event> nextEvent;
		boost::mutex eventLock;
    public:       
        /**
         * Constructor.  Defines an Variable member and the event type.
         */
        Event(const int _code, const MonkeyWorksTime _time, const Data &data);
		Event(const int _code, const Data &data);
		Event(ScarabDatum *datum);  // create an event from a ScarabDatum
        Event(){ } // for construction during deserialization
        virtual ~Event() {}
		
        /**
         * Returns the event code.  This code is given to an
         * Variable by the parameter registry.
         */
        int getEventCode(){ 
			return code;
		}
		
        /**
         * Returns the event time.
         */
        MonkeyWorksTime getTime(){ 
			return time;
		}
		
		Data getData() {
			return data;
		}
		
		shared_ptr<Event> getNextEvent() {
			boost::mutex::scoped_lock lock(eventLock);
			return nextEvent;
		}
		
		void setNextEvent(shared_ptr<Event> _nextEvent) {
			boost::mutex::scoped_lock lock(eventLock);
			nextEvent = _nextEvent;
		}
		
		
		
        // Convert to ScarabDatum for later serialization
        virtual ScarabDatum *toScarabDatum();
		
        // Boost serialization
        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive & ar, const unsigned int version){
            ar & code;
            ar & time;
            ar & data;
        }
	};
	
}
#endif

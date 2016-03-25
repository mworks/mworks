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

#include "MWorksTypes.h"
#include "GenericData.h"
#include "EventConstants.h"
#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>

#include <boost/serialization/list.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/version.hpp>

using boost::shared_ptr;


BEGIN_NAMESPACE_MW


class Event {
    
public:
    class Buffer;
    
    Event() :
        code(M_UNDEFINED_EVENT_CODE),
        time(0)
    { }
    
    Event(int code, const Datum &data);
    
    Event(int code, MWTime time, const Datum &data) :
        code(code),
        time(time),
        data(data)
    { }
    
    Event(const Event &other) :
        Event(other.code, other.time, other.data)
    { }
    
    int getEventCode() const {
        return code;
    }
    
    MWTime getTime() const {
        return time;
    }
    
    const Datum& getData() const {
        return data;
    }
    
    shared_ptr<Event> getNextEvent() const {
        boost::mutex::scoped_lock lock(eventLock);
        return nextEvent;
    }
    
private:
    void setNextEvent(const shared_ptr<Event> &event) {
        boost::mutex::scoped_lock lock(eventLock);
        nextEvent = event;
    }
    
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar & code;
        ar & time;
        ar & data;
    }
    
    int code;
    MWTime time;
    Datum data;
    
    shared_ptr<Event> nextEvent;
    mutable boost::mutex eventLock;
    
};


// An abstract protocol for objects that can receive events
class EventReceiver {
public:
    virtual ~EventReceiver() { }
    virtual void putEvent(const shared_ptr<Event> &event) = 0;
};


class Event::Buffer : public EventReceiver {
    
public:
    Buffer() :
        headEvent(new Event)
    { }
    
    void putEvent(const shared_ptr<Event> &event) override {
        boost::mutex::scoped_lock lock(bufferLock);
        headEvent->setNextEvent(event);
        headEvent = event;
    }
    
    shared_ptr<Event> getHeadEvent() const {
        boost::mutex::scoped_lock lock(bufferLock);
        return headEvent;
    }
    
private:
    shared_ptr<Event> headEvent;
    mutable boost::mutex bufferLock;
    
};


END_NAMESPACE_MW


#endif




























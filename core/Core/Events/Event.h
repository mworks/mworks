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

#include <condition_variable>
#include <mutex>

#include <boost/serialization/list.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/version.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>

#include "MWorksTypes.h"
#include "GenericData.h"
#include "EventConstants.h"

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
    
    boost::shared_ptr<Event> getNextEvent(MWTime timeoutUS = 0) const;
    
private:
    void setNextEvent(const boost::shared_ptr<Event> &event);
    
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
    
    boost::shared_ptr<Event> nextEvent;
    
    mutable std::mutex mutex;
    using lock_guard = std::lock_guard<decltype(mutex)>;
    mutable std::condition_variable nextEventAvailable;
    
};


// An abstract protocol for objects that can receive events
class EventReceiver {
public:
    virtual ~EventReceiver() { }
    virtual void putEvent(const boost::shared_ptr<Event> &event) = 0;
};


class Event::Buffer : public EventReceiver {
    
public:
    Buffer() :
        headEvent(new Event)
    { }
    
    void putEvent(const boost::shared_ptr<Event> &event) override {
        lock_guard lock(mutex);
        headEvent->setNextEvent(event);
        headEvent = event;
    }
    
    boost::shared_ptr<Event> getHeadEvent() const {
        lock_guard lock(mutex);
        return headEvent;
    }
    
private:
    boost::shared_ptr<Event> headEvent;
    mutable std::mutex mutex;
    
};


END_NAMESPACE_MW


#endif




























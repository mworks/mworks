/*
 *  DateFileIndexer.h
 *  DateFileIndexer
 *
 *  Created by bkennedy on 2/19/08.
 *  Copyright 2008 MIT. All rights reserved.
 *
 */

#ifndef DateFileIndexer_
#define DateFileIndexer_

#include <set>
#include <stdexcept>
#include <vector>

#include <boost/filesystem/path.hpp>
#include <boost/format.hpp>
#include <boost/noncopyable.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/shared_ptr.hpp>

#include <MWorksCore/Utilities.h>

#include "EventWrapper.h"
#include "EventBlock.h"


class DataFileIndexerError : public std::runtime_error {
    
public:
    explicit DataFileIndexerError(const std::string &s) :
        std::runtime_error(s)
    { }
    
    explicit DataFileIndexerError(const boost::format &f) :
        std::runtime_error(f.str())
    { }
    
};


class DataFileIndexer : boost::noncopyable {
    
public:
    ~DataFileIndexer();
    
    DataFileIndexer(const boost::filesystem::path &data_file);
    
    void buildIndex(unsigned int events_per_block, unsigned int multiplication_factor_per_level);
    
    unsigned int getNEvents() const { return number_of_events; }
    MWTime getMinimumTime() const { return root->minimumTime(); }
    MWTime getMaximumTime() const { return root->maximumTime(); }
    
    void getEvents(std::vector<EventWrapper> &events,
                   const std::set<unsigned int> &event_codes,
                   MWTime lower_bound,
                   MWTime upper_bound) const;
    
    class EventsIterator {
    private:
        const DataFileIndexer &dfi;
        const std::set<unsigned int> event_codes_to_match;
        const MWTime lower_bound;
        const MWTime upper_bound;
        
        std::vector<boost::shared_ptr<EventBlock> > matching_event_blocks;
        std::vector<boost::shared_ptr<EventBlock> >::size_type current_event_block;
        
        unsigned int current_relative_event;
        ScarabDatum *current_datum;
        
    public:
        EventsIterator(const DataFileIndexer &indexer,
                       const std::set<unsigned int> &event_codes,
                       MWTime lower_bound,
                       MWTime upper_bound);
        
        EventWrapper getNextEvent();
    };
    
    EventsIterator getEventsIterator(const std::set<unsigned int> &event_codes,
                                     MWTime lower_bound,
                                     MWTime upper_bound) const
    {
        return EventsIterator(*this, event_codes, lower_bound, upper_bound);
    }
    
private:
    friend class boost::serialization::access;
    
    template<class Archive> void save(Archive & ar, const unsigned int version) const {
        ar << number_of_events;
        ar << events_per_block;
        ar << root;
    }
    
    template<class Archive> void load(Archive & ar, const unsigned int version) {
        if (version == 0) {
            // Version 0 stored the URI.  We don't anymore, so just read and discard it.
            std::string uri;
            ar >> uri;
        }
        ar >> number_of_events;
        ar >> events_per_block;
        ar >> root;
    }
    
    BOOST_SERIALIZATION_SPLIT_MEMBER()
    
    ScarabSession *session;
    unsigned int number_of_events;
    unsigned int events_per_block;
    
    boost::shared_ptr<EventBlock> root;
    
};


BOOST_CLASS_VERSION(DataFileIndexer, 1)


#endif  // !defined(DateFileIndexer_)




























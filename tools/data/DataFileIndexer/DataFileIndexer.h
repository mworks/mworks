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

/* The classes below are exported */
#pragma GCC visibility push(default)

#include <vector>
#include <map>
#include "boost/filesystem/path.hpp"
#include "boost/shared_ptr.hpp"
//#include "Scarab/scarab.h"
#include <MWorksCore/Utilities.h>
using namespace mw;
#include "EventWrapper.h"
#include "EventBlock.h"
#include "boost/archive/text_oarchive.hpp"
#include "boost/archive/text_iarchive.hpp"
#include "boost/serialization/split_member.hpp"


class DataFileIndexer {
    
private:
    friend class boost::serialization::access;
    template<class Archive> void save(Archive & ar, const unsigned int version) const {
        ar << uri;
        ar << number_of_events;
        ar << events_per_block;
        ar << root;
    }
    
    template<class Archive> void load(Archive & ar, const unsigned int version)	{
        ar >> uri;
        ar >> number_of_events;
        ar >> events_per_block;
        ar >> root;
    }
    
    BOOST_SERIALIZATION_SPLIT_MEMBER();
    
    ScarabSession *session;
    std::string uri;
    unsigned int number_of_events;
    unsigned int events_per_block;
    
    boost::shared_ptr<EventBlock> root;
    
public:
    DataFileIndexer();
    DataFileIndexer(const boost::filesystem::path &data_file,
                    const unsigned int events_per_block = 1000,
                    const unsigned int multiplication_factor_per_level = 2,
                    const int number_of_indexing_threads = 1);
    ~DataFileIndexer();
    
    void openScarabSession(const boost::filesystem::path &data_file) {
        uri = "ldobinary:file_readonly://" + data_file.string();
        session = scarab_session_connect(uri.c_str());
    }
    
    void reconstituteScarabSession(const boost::filesystem::path &data_file){
        boost::filesystem::path true_mwk_path = data_file / data_file.leaf();
        openScarabSession(true_mwk_path);
    }
    
    unsigned int getNEvents() const { return number_of_events; }
    MWTime getMinimumTime() const { return root->minimumTime(); }
    MWTime getMaximumTime() const { return root->maximumTime(); }
    
    void getEvents(std::vector<EventWrapper> &events,
                   const std::vector<unsigned int> &event_codes,
                   const MWTime lower_bound = MIN_MONKEY_WORKS_TIME(),
                   const MWTime upper_bound = MAX_MONKEY_WORKS_TIME()) const;
    
    class EventsIterator {
    private:
        const DataFileIndexer &dfi;
        const std::vector<unsigned int> event_codes_to_match;
        const MWTime lower_bound;
        const MWTime upper_bound;
        
        std::vector<boost::shared_ptr<EventBlock> > matching_event_blocks;
        std::vector<boost::shared_ptr<EventBlock> >::const_iterator matching_event_blocks_iter;
        
        unsigned int current_relative_event;
        ScarabDatum *current_datum;
        
    public:
        EventsIterator(const DataFileIndexer &indexer,
                       const std::vector<unsigned int> &event_codes,
                       MWTime lower_bound,
                       MWTime upper_bound);
        
        EventWrapper getNextEvent();
    };
    
    EventsIterator getEventsIterator(const std::vector<unsigned int> &event_codes,
                                     const MWTime lower_bound = MIN_MONKEY_WORKS_TIME(),
                                     const MWTime upper_bound = MAX_MONKEY_WORKS_TIME()) const
    {
        return EventsIterator(*this, event_codes, lower_bound, upper_bound);
    }
    
};

#pragma GCC visibility pop
#endif




























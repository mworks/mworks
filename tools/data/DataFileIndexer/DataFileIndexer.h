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

#include <vector>

#include <boost/filesystem/path.hpp>
#include <boost/serialization/access.hpp>
#include <boost/shared_ptr.hpp>

#include <MWorksCore/Utilities.h>

#include "EventWrapper.h"
#include "EventBlock.h"


class DataFileIndexer {
    
private:
    friend class boost::serialization::access;
    template<class Archive> void serialize(Archive & ar, const unsigned int version) {
        ar & uri;
        ar & number_of_events;
        ar & events_per_block;
        ar & root;
    }
    
    ScarabSession *session;
    std::string uri;
    unsigned int number_of_events;
    unsigned int events_per_block;
    
    boost::shared_ptr<EventBlock> root;
    
public:
    DataFileIndexer();
    explicit DataFileIndexer(const boost::filesystem::path &data_file,
                             unsigned int events_per_block = 5000,
                             unsigned int multiplication_factor_per_level = 4);
    ~DataFileIndexer();
    
    void openScarabSession(const boost::filesystem::path &data_file);
    
    void reconstituteScarabSession(const boost::filesystem::path &data_file){
        boost::filesystem::path true_mwk_path = data_file / data_file.leaf();
        openScarabSession(true_mwk_path);
    }
    
    unsigned int getNEvents() const { return number_of_events; }
    MWTime getMinimumTime() const { return root->minimumTime(); }
    MWTime getMaximumTime() const { return root->maximumTime(); }
    
    void getEvents(std::vector<EventWrapper> &events,
                   const std::vector<unsigned int> &event_codes,
                   MWTime lower_bound,
                   MWTime upper_bound) const;
    
    class EventsIterator {
    private:
        const DataFileIndexer &dfi;
        const std::vector<unsigned int> event_codes_to_match;
        const MWTime lower_bound;
        const MWTime upper_bound;
        
        std::vector<boost::shared_ptr<EventBlock> > matching_event_blocks;
        std::vector<boost::shared_ptr<EventBlock> >::size_type current_event_block;
        
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
                                     MWTime lower_bound,
                                     MWTime upper_bound) const
    {
        return EventsIterator(*this, event_codes, lower_bound, upper_bound);
    }
    
};


#endif  // !defined(DateFileIndexer_)




























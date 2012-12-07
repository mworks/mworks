/*
 *  DataFileIndexer.cp
 *  DataFileIndexer
 *
 *  Created by bkennedy on 2/19/08.
 *  Copyright 2008 MIT. All rights reserved.
 *
 */

#include "DataFileIndexer.h"
#include "DataFileUtilities.h"
#include <boost/shared_ptr.hpp>
#include <algorithm>
#include <iostream>


DataFileIndexer::~DataFileIndexer() {
    scarab_session_close(session);
}


DataFileIndexer::DataFileIndexer(const boost::filesystem::path &data_file) :
    number_of_events(0),
    events_per_block(0)
{
    const std::string uri("ldobinary:file_readonly://" + data_file.string());
    session = scarab_session_connect(uri.c_str());
    if ((session == NULL) || (scarab_session_geterr(session) != 0)) {
        scarab_mem_free(session);
		throw DataFileIndexerError(boost::format("Cannot open file \"%s\"") % data_file.string());
    }
}


void DataFileIndexer::buildIndex(unsigned int _events_per_block, unsigned int multiplication_factor_per_level) {
    number_of_events = 0;
    events_per_block = _events_per_block;
    root.reset();
	
	{
		std::vector<boost::shared_ptr<EventBlock> > event_blocks;
		{
			std::set<unsigned int> event_codes_in_block;
			MWTime max_time = MIN_MWORKS_TIME();
			MWTime min_time = MAX_MWORKS_TIME();
			long int previous_datum_location = scarab_tell(session);
			
			ScarabDatum *datum = NULL;
			while(datum = scarab_read(session)) {
                if (!DataFileUtilities::isScarabEvent(datum)) {
                    // Ignore invalid events
                    scarab_free_datum(datum);
                    continue;
                }
                
				event_codes_in_block.insert(DataFileUtilities::getScarabEventCode(datum));
				
				MWTime event_time = DataFileUtilities::getScarabEventTime(datum);
				max_time = std::max(max_time, event_time);
				min_time = std::min(min_time, event_time);
				
				number_of_events++;
				if(number_of_events % events_per_block == 0) {
					boost::shared_ptr<EventBlock> new_event_block = boost::shared_ptr<EventBlock>(new EventBlock(previous_datum_location, min_time, max_time, event_codes_in_block));
					event_blocks.push_back(new_event_block);
					
					event_codes_in_block.clear();
					max_time = MIN_MWORKS_TIME();
					min_time = MAX_MWORKS_TIME();
					previous_datum_location = scarab_tell(session);
				}			
				scarab_free_datum(datum);
			}
			
			// add in the remainder blocks
			boost::shared_ptr<EventBlock> new_event_block = boost::shared_ptr<EventBlock>(new EventBlock(previous_datum_location, min_time, max_time, event_codes_in_block));
			event_blocks.push_back(new_event_block);
		}
		
		
		{
			// build the tree
			int events_per_node = events_per_block;
			int number_of_levels = 1;
			while(events_per_node < number_of_events) {
				number_of_levels++;
				events_per_node *= multiplication_factor_per_level;
			}
			
			std::vector <boost::shared_ptr<EventBlock> > blocks_at_next_level = event_blocks;
			for(int i = 1; i < number_of_levels; ++i) {
				std::vector <boost::shared_ptr<EventBlock> > blocks_at_current_level;
				
				std::vector<boost::shared_ptr<EventBlock> >::const_iterator j = blocks_at_next_level.begin();
				while(j != blocks_at_next_level.end()) {
					std::vector <boost::shared_ptr<EventBlock> > children;					
					for(int k = 0; k < multiplication_factor_per_level && j != blocks_at_next_level.end(); ++k) {
						children.push_back(*j);
						++j;
					}
					boost::shared_ptr<EventBlock> new_block = boost::shared_ptr<EventBlock>(new EventBlock(children));
					blocks_at_current_level.push_back(new_block);
				}
				
				blocks_at_next_level = blocks_at_current_level;
			}
			
			if(blocks_at_next_level.size() != 1) {
				//badness
				throw DataFileIndexerError("Something went wrong ... please abort and try again");
			}
            
            // DDC added a patch to fix failure to index small numbers of events
            // force a mandatory level below root
            boost::shared_ptr<EventBlock> subroot = blocks_at_next_level.at(0);
            std::vector< boost::shared_ptr<EventBlock> > root_children;
            root_children.push_back(subroot);
			root = boost::shared_ptr<EventBlock>(new EventBlock(root_children));
        }
	}
}


void DataFileIndexer::getEvents(std::vector<EventWrapper> &return_vector,
                                const std::set<unsigned int> &event_codes_to_match,
                                MWTime lower_bound,
                                MWTime upper_bound) const
{
    EventsIterator ei = getEventsIterator(event_codes_to_match, lower_bound, upper_bound);
    while (true) {
        EventWrapper event = ei.getNextEvent();
        if (event.empty())
            break;
        return_vector.push_back(event);
    }
}


DataFileIndexer::EventsIterator::EventsIterator(const DataFileIndexer &_dfi,
                                                const std::set<unsigned int> &_event_codes_to_match,
                                                MWTime _lower_bound,
                                                MWTime _upper_bound) :
    dfi(_dfi),
    event_codes_to_match(_event_codes_to_match),
    lower_bound(_lower_bound),
    upper_bound(_upper_bound)
{
    if (lower_bound > upper_bound) {
        throw DataFileIndexerError("Minimum event time must be less than or equal to maximum event time");
    }
    
    // Recursively find event blocks that meet our search criteria
	dfi.root->children(matching_event_blocks, event_codes_to_match, lower_bound, upper_bound);
    
    // Prepare for iteration
    current_event_block = 0;
    current_relative_event = dfi.events_per_block;
    current_datum = NULL;
}


EventWrapper DataFileIndexer::EventsIterator::getNextEvent() {
    EventWrapper event;
    
    while (current_event_block < matching_event_blocks.size()) {
        if ((current_relative_event == dfi.events_per_block) || (current_datum == NULL)) {
            // Advance to the next block
            scarab_seek(dfi.session, matching_event_blocks[current_event_block]->blockOffset(), SEEK_SET);
            current_relative_event = 0;
        }
        
        // Read through the event block
		while (event.empty() && (current_relative_event < dfi.events_per_block) && (current_datum = scarab_read(dfi.session)))
        {
            if (!DataFileUtilities::isScarabEvent(current_datum)) {
                // Skip invalid events
                scarab_free_datum(current_datum);
                continue;
            }
            
			MWTime event_time = DataFileUtilities::getScarabEventTime(current_datum);
			
            // Check the time criterion
            if (event_time >= lower_bound && event_time <= upper_bound) {
				unsigned int event_code = DataFileUtilities::getScarabEventCode(current_datum);
				
                // Check if the event code matches
                if (event_codes_to_match.empty() ||
                    (event_codes_to_match.find(event_code) != event_codes_to_match.end()))
                {
                    event = EventWrapper(current_datum);
                }
			}
			
			scarab_free_datum(current_datum);
			current_relative_event++;
		}
        
        if ((current_relative_event == dfi.events_per_block) || (current_datum == NULL))
            current_event_block++;
        
        if (!event.empty())
            return event;
    }
    
    return event;
}



























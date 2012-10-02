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
#include "boost/shared_ptr.hpp"
#include <algorithm>
#include <iostream>

DataFileIndexer::DataFileIndexer() {
	session = 0;
}

DataFileIndexer::DataFileIndexer(const boost::filesystem::path &data_file, 
								 const unsigned int _events_per_block,
								 const unsigned int multiplication_factor_per_level,
								 const int number_of_indexing_threads) :
    events_per_block(_events_per_block)
{
    openScarabSession(data_file);
	
	{
		std::vector<boost::shared_ptr<EventBlock> > event_blocks;
		{
			number_of_events = 0;
			
			std::vector<unsigned int> event_codes_in_block;
			MWTime max_time = MIN_MONKEY_WORKS_TIME();
			MWTime min_time = MAX_MONKEY_WORKS_TIME();
			long int previous_datum_location = scarab_tell(session);
			
			ScarabDatum *datum = NULL;
			while(datum = scarab_read(session)) {
				event_codes_in_block.push_back(DataFileUtilities::getScarabEventCode(datum));
				
				const MWTime event_time = DataFileUtilities::getScarabEventTime(datum);
				max_time = max_time > event_time ? max_time : event_time;
				min_time = min_time < event_time ? min_time : event_time;
				
				
				number_of_events++;
				if(number_of_events % events_per_block == 0) {
					std::sort(event_codes_in_block.begin(), event_codes_in_block.end());
					event_codes_in_block.erase(std::unique(event_codes_in_block.begin(), 
														   event_codes_in_block.end()), 
											   event_codes_in_block.end());
					//std::cout << "indexing block " << event_blocks.size() << " .. time : " << min_time << "LL - " << max_time << "LL" << "\r";

					//				cerr << "new event block : num events : " << event_codes_in_block.size() << endl;
					boost::shared_ptr<EventBlock> new_event_block = boost::shared_ptr<EventBlock>(new EventBlock(previous_datum_location, min_time, max_time, event_codes_in_block));
					event_blocks.push_back(new_event_block);
					
					event_codes_in_block.clear();
					max_time = MIN_MONKEY_WORKS_TIME();
					min_time = MAX_MONKEY_WORKS_TIME();
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
				std::cerr << "something went wrong...please abort and try again" << std::endl;
				throw new std::exception;
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

DataFileIndexer::~DataFileIndexer() {
	if(session) {
		scarab_session_close(session);
	}
}

void DataFileIndexer::getEvents(std::vector<EventWrapper> &return_vector,
                                const std::vector<unsigned int> &event_codes_to_match,
                                const MWTime lower_bound,
                                const MWTime upper_bound) const
{
    EventsIterator ei = getEventsIterator(event_codes_to_match, lower_bound, upper_bound);
    EventWrapper event;
    while ((event = ei.getNextEvent())) {
        return_vector.push_back(event);
    }
}


DataFileIndexer::EventsIterator::EventsIterator(const DataFileIndexer &_dfi,
                                                const std::vector<unsigned int> &_event_codes_to_match,
                                                MWTime _lower_bound,
                                                MWTime _upper_bound) :
    dfi(_dfi),
    event_codes_to_match(_event_codes_to_match),
    lower_bound(_lower_bound),
    upper_bound(_upper_bound)
{
    // Recursively find event blocks that meet our search criteria
	dfi.root->children(matching_event_blocks, event_codes_to_match, lower_bound, upper_bound);
    
    // Prepare for iteration
    matching_event_blocks_iter = matching_event_blocks.begin();
    current_relative_event = dfi.events_per_block;
    current_datum = NULL;
}


EventWrapper DataFileIndexer::EventsIterator::getNextEvent() {
    EventWrapper event;
    
    while (matching_event_blocks_iter != matching_event_blocks.end()) {
        if ((current_relative_event == dfi.events_per_block) || (current_datum == NULL)) {
            // Advance to the next block
            scarab_seek(dfi.session, (*matching_event_blocks_iter)->blockOffset(), SEEK_SET);
            current_relative_event = 0;
        }
        
        // Read through the event block
		while (!event && (current_relative_event < dfi.events_per_block) && (current_datum = scarab_read(dfi.session)))
        {
			MWTime event_time = DataFileUtilities::getScarabEventTime(current_datum);
			
            // Check the time criterion
            if (event_time >= lower_bound && event_time <= upper_bound) {
				unsigned int event_code = DataFileUtilities::getScarabEventCode(current_datum);
				
                // Check if the event code matches
                if (event_codes_to_match.size() == 0 ||
                    std::find(event_codes_to_match.begin(), event_codes_to_match.end(), event_code) != event_codes_to_match.end())
                {
                    event = EventWrapper(current_datum);
                }
			}
			
			scarab_free_datum(current_datum);
			current_relative_event++;
		}
        
        if ((current_relative_event == dfi.events_per_block) || (current_datum == NULL))
            matching_event_blocks_iter++;
        
        if (event)
            return event;
    }
    
    return event;
}



























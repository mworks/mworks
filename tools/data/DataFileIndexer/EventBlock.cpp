/*
 *  EventBlock.cpp
 *  DataFileIndexer
 *
 *  Created by bkennedy on 2/19/08.
 *  Copyright 2008 MIT. All rights reserved.
 *
 */

#include <algorithm>
#include <iostream>

#include "EventBlock.h"

EventBlock::EventBlock(long int offset,
					   MWTime min_time,
					   MWTime max_time,
					   const std::set<unsigned int> &_event_codes) :
file_offset(offset), 
minimum_time(min_time), 
maximum_time(max_time), 
event_codes(_event_codes) {
}


EventBlock::EventBlock(const std::vector<boost::shared_ptr<EventBlock> > &child_event_blocks) :
file_offset(-1), 
minimum_time(MAX_MWORKS_TIME()), 
maximum_time(MIN_MWORKS_TIME())
{
	for(std::vector<boost::shared_ptr<EventBlock> >::const_iterator i = child_event_blocks.begin();
		i != child_event_blocks.end();
		++i) {
		addChild(*i);
	}
}


bool EventBlock::hasEventCodes(const std::set<unsigned int> &event_codes_to_match) const {
    // if no codes are passed in, assume that any code is okay
    if (event_codes_to_match.empty()) {
        return true;
    }
    
    // otherwise, go through the codes contained in this block to determine acceptance
    for (std::set<unsigned int>::const_iterator j = event_codes_to_match.begin();
		j != event_codes_to_match.end();
		++j) {
		if(hasEventCode(*j)) {
			return true;
		}
	}
    
	return false;
}


void EventBlock::addChild(const boost::shared_ptr<EventBlock> &child) {
	_children.push_back(child);
	
	maximum_time = std::max(maximum_time, child->maximumTime());
	minimum_time = std::min(minimum_time, child->minimumTime());
    
    event_codes.insert(child->eventCodes().begin(), child->eventCodes().end());
}


void EventBlock::children(std::vector<boost::shared_ptr<EventBlock> > &matching_child_blocks,
                          const std::set<unsigned int> &event_codes_to_match,
                          MWTime lower_bound,
                          MWTime upper_bound) const
{
	for(std::vector<boost::shared_ptr<EventBlock> >::const_iterator i = _children.begin();
		i != _children.end();
		++i) {
		
		if((*i)->hasTime(lower_bound, upper_bound) && (*i)->hasEventCodes(event_codes_to_match)) {
			if((*i)->isLeaf()) {
				matching_child_blocks.push_back(*i);
			} else {
				(*i)->children(matching_child_blocks, event_codes_to_match, lower_bound, upper_bound);
			}
		}
	}
}


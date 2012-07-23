/*
 *  EventBlock.cpp
 *  DataFileIndexer
 *
 *  Created by bkennedy on 2/19/08.
 *  Copyright 2008 MIT. All rights reserved.
 *
 */

#include <iostream>

#include "EventBlock.h"

EventBlock::EventBlock(const long int offset,
					   const MWorksTime min_time,
					   const MWorksTime max_time,
					   const std::vector<unsigned int> _event_codes) : 
file_offset(offset), 
minimum_time(min_time), 
maximum_time(max_time), 
event_codes(_event_codes) {
}

EventBlock::EventBlock(const std::vector<boost::shared_ptr<EventBlock> > child_event_blocks) :
file_offset(-1), 
minimum_time(MAX_MONKEY_WORKS_TIME()), 
maximum_time(MIN_MONKEY_WORKS_TIME())
{
	for(std::vector<boost::shared_ptr<EventBlock> >::const_iterator i = child_event_blocks.begin();
		i != child_event_blocks.end();
		++i) {
		addChild(*i);
	}
}

bool EventBlock::hasTime(const MWorksTime lower_bound,
						 const MWorksTime upper_bound) const {
	return lower_bound <= maximum_time && upper_bound >= minimum_time;
}

bool EventBlock::hasEventCode(const unsigned int event_code) const {
	for(std::vector<unsigned int>::const_iterator i = event_codes.begin();
		i != event_codes.end();
		++i) {
		if(*i == event_code) {
			return true;
		}
	}
	
	return false;
}

bool EventBlock::hasEventCodes(const std::vector<unsigned int> &event_codes_to_match) const {
	
    // if no codes are passed in, assume that any code is okay
    if(event_codes_to_match.size() == 0){
        return true;
    }
    
    
    // otherwise, go through the codes contained in this block to determine acceptance
    for(std::vector<unsigned int>::const_iterator j = event_codes_to_match.begin();
		j != event_codes_to_match.end();
		++j) {
		if(hasEventCode(*j)) {
			return true;
		}
	}
	return false;
}

bool EventBlock::isLeaf() const {
	return _children.size() == 0;
}

void EventBlock::addChild(boost::shared_ptr<EventBlock> child) {
	_children.push_back(child);
	
	maximum_time = maximum_time > child->maximumTime() ? maximum_time : child->maximumTime();
	minimum_time = minimum_time < child->minimumTime() ? minimum_time : child->minimumTime();
	
	std::vector<unsigned int> new_event_codes = child->eventCodes();
	for(std::vector<unsigned int>::const_iterator i = new_event_codes.begin();
		i != new_event_codes.end();
		++i) {
		event_codes.push_back(*i);
	}

	std::sort(event_codes.begin(), event_codes.end());
	event_codes.erase(std::unique(event_codes.begin(), 
								  event_codes.end()), 
					  event_codes.end());
}

MWorksTime EventBlock::maximumTime() const {
	return maximum_time;
}

MWorksTime EventBlock::minimumTime() const {
	return minimum_time;
}

long int EventBlock::blockOffset() const {
	return file_offset;
}

std::vector<unsigned int> EventBlock::eventCodes() const {
	return event_codes;
}

std::vector<boost::shared_ptr<EventBlock> > EventBlock::children(const std::vector<unsigned int> &event_codes_to_match,
																 const MWorksTime lower_bound,
																 const MWorksTime upper_bound) const {
	std::vector<boost::shared_ptr<EventBlock> > matching_child_blocks;
	
	for(std::vector<boost::shared_ptr<EventBlock> >::const_iterator i = _children.begin();
		i != _children.end();
		++i) {
		
		if((*i)->hasTime(lower_bound, upper_bound) && (*i)->hasEventCodes(event_codes_to_match)) {
			if((*i)->isLeaf()) {
				matching_child_blocks.push_back(*i);
			} else {
				std::vector<boost::shared_ptr<EventBlock> > lower_matching_child_blocks = (*i)->children(event_codes_to_match, lower_bound, upper_bound);
				for(std::vector<boost::shared_ptr<EventBlock> >::const_iterator j = lower_matching_child_blocks.begin();
					j != lower_matching_child_blocks.end();
					++j) {
					if((*j)->isLeaf()) {
						matching_child_blocks.push_back(*j);
					} else {
						std::cerr << "shouldn't be here" << std::endl;
					}
				}
			}
		}
	}
	
	return matching_child_blocks;
}


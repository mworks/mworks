/*
 *  EventBlock.h
 *  DataFileIndexer
 *
 *  Created by bkennedy on 2/19/08.
 *  Copyright 2008 MIT. All rights reserved.
 *
 */

#ifndef EventBlock_
#define EventBlock_

#include "MonkeyWorksCore.h"
#include <vector>
#include "boost/shared_ptr.hpp"
#include "boost/archive/text_oarchive.hpp"
#include "boost/archive/text_iarchive.hpp"
#include "boost/serialization/shared_ptr.hpp"
#include "boost/serialization/vector.hpp"



class EventBlock {
protected:
	friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & file_offset;
        ar & minimum_time;
        ar & maximum_time;
		ar & event_codes;
		ar & _children;
    }
	
	long int file_offset;
	MonkeyWorksTime minimum_time;
	MonkeyWorksTime maximum_time;
	std::vector<unsigned int> event_codes;
	std::vector<boost::shared_ptr<EventBlock> > _children;
	
public:
	EventBlock() {};
	EventBlock(const long int offset,
			   const MonkeyWorksTime min_time,
			   const MonkeyWorksTime max_time,
			   const std::vector<unsigned int> _event_codes);
	
	EventBlock(const std::vector<boost::shared_ptr<EventBlock> > child_event_blocks);
	
	bool hasTime(const MonkeyWorksTime lower_bound, 
				 const MonkeyWorksTime upper_bound) const;
	bool hasEventCode(const unsigned int event_code) const;
	bool hasEventCodes(const std::vector<unsigned int> &event_codes_to_match) const;
	bool isLeaf() const;
	void addChild(boost::shared_ptr<EventBlock> child);	
	MonkeyWorksTime maximumTime() const;
	MonkeyWorksTime minimumTime() const;
	long int blockOffset() const;
	std::vector<unsigned int> eventCodes() const;
	std::vector<boost::shared_ptr<EventBlock> > children(const std::vector<unsigned int> &event_codes,
														 const MonkeyWorksTime lower_bound,
														 const MonkeyWorksTime upper_bound) const;	
};

#endif // EventBlock_

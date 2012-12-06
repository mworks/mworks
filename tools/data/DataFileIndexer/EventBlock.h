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

#include <MWorksCore/Utilities.h>
#include <vector>
#include "boost/shared_ptr.hpp"
#include "boost/archive/text_oarchive.hpp"
#include "boost/archive/text_iarchive.hpp"
#include "boost/serialization/shared_ptr.hpp"
#include "boost/serialization/vector.hpp"

using mw::MWTime;
using mw::MIN_MWORKS_TIME;
using mw::MAX_MWORKS_TIME;



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
	MWTime minimum_time;
	MWTime maximum_time;
	std::vector<unsigned int> event_codes;
	std::vector<boost::shared_ptr<EventBlock> > _children;
	
public:
	EventBlock() {}
	EventBlock(long int offset,
			   MWTime min_time,
			   MWTime max_time,
			   const std::vector<unsigned int> &_event_codes);
	
	EventBlock(const std::vector<boost::shared_ptr<EventBlock> > &child_event_blocks);
	
	bool hasTime(MWTime lower_bound, 
				 MWTime upper_bound) const;
	bool hasEventCode(unsigned int event_code) const;
	bool hasEventCodes(const std::vector<unsigned int> &event_codes_to_match) const;
	bool isLeaf() const { return _children.size() == 0; }
	void addChild(const boost::shared_ptr<EventBlock> &child);
	MWTime maximumTime() const { return maximum_time; }
	MWTime minimumTime() const { return minimum_time; }
	long int blockOffset() const { return file_offset; }
	const std::vector<unsigned int>& eventCodes() const { return event_codes; }
	void children(std::vector<boost::shared_ptr<EventBlock> > &matching_child_blocks,
                  const std::vector<unsigned int> &event_codes,
                  MWTime lower_bound,
                  MWTime upper_bound) const;	
};

#endif // EventBlock_



























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
using namespace mw;
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
	MWTime minimum_time;
	MWTime maximum_time;
	std::vector<unsigned int> event_codes;
	std::vector<boost::shared_ptr<EventBlock> > _children;
	
public:
	EventBlock() {};
	EventBlock(const long int offset,
			   const MWTime min_time,
			   const MWTime max_time,
			   const std::vector<unsigned int> _event_codes);
	
	EventBlock(const std::vector<boost::shared_ptr<EventBlock> > child_event_blocks);
	
	bool hasTime(const MWTime lower_bound, 
				 const MWTime upper_bound) const;
	bool hasEventCode(const unsigned int event_code) const;
	bool hasEventCodes(const std::vector<unsigned int> &event_codes_to_match) const;
	bool isLeaf() const;
	void addChild(boost::shared_ptr<EventBlock> child);	
	MWTime maximumTime() const;
	MWTime minimumTime() const;
	long int blockOffset() const;
	std::vector<unsigned int> eventCodes() const;
	void children(std::vector<boost::shared_ptr<EventBlock> > &matching_child_blocks,
                  const std::vector<unsigned int> &event_codes,
                  const MWTime lower_bound,
                  const MWTime upper_bound) const;	
};

#endif // EventBlock_

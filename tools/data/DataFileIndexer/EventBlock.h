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

#include <set>
#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/extended_type_info.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/singleton.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/vector.hpp>

#include <MWorksCore/Utilities.h>

using mw::MWTime;
using mw::MIN_MWORKS_TIME;
using mw::MAX_MWORKS_TIME;


class EventBlock {
	
public:
	EventBlock() {}
	EventBlock(long int offset,
			   MWTime min_time,
			   MWTime max_time,
			   const std::set<unsigned int> &_event_codes);
	
	EventBlock(const std::vector<boost::shared_ptr<EventBlock> > &child_event_blocks);
	
	bool hasTime(MWTime lower_bound, MWTime upper_bound) const {
        return lower_bound <= maximum_time && upper_bound >= minimum_time;
    }
	bool hasEventCode(unsigned int code) const { return (event_codes.find(code) != event_codes.end()); }
	bool hasEventCodes(const std::set<unsigned int> &event_codes_to_match) const;
	bool isLeaf() const { return _children.empty(); }
	void addChild(const boost::shared_ptr<EventBlock> &child);
	MWTime maximumTime() const { return maximum_time; }
	MWTime minimumTime() const { return minimum_time; }
	long int blockOffset() const { return file_offset; }
	const std::set<unsigned int>& eventCodes() const { return event_codes; }
	void children(std::vector<boost::shared_ptr<EventBlock> > &matching_child_blocks,
                  const std::set<unsigned int> &event_codes,
                  MWTime lower_bound,
                  MWTime upper_bound) const;	

private:
    friend class boost::serialization::access;
    
    template<class Archive> void save(Archive & ar, const unsigned int version) const {
        ar << file_offset;
        ar << minimum_time;
        ar << maximum_time;
        ar << event_codes;
        ar << _children;
    }
    
    template<class Archive> void load(Archive & ar, const unsigned int version) {
        ar >> file_offset;
        ar >> minimum_time;
        ar >> maximum_time;
        if (version > 0) {
            ar >> event_codes;
        } else {
            std::vector<unsigned int> codes;
            ar >> codes;
            event_codes.clear();
            event_codes.insert(codes.begin(), codes.end());
        }
        ar >> _children;
    }
    
    BOOST_SERIALIZATION_SPLIT_MEMBER()
    
    long int file_offset;
    MWTime minimum_time;
    MWTime maximum_time;
    std::set<unsigned int> event_codes;
    std::vector<boost::shared_ptr<EventBlock> > _children;

};


BOOST_CLASS_VERSION(EventBlock, 1)


#endif // EventBlock_



























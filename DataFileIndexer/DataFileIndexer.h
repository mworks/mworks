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
#include "MonkeyWorksCore.h"
#include "ScarabDatumWrapper.h"
#include "EventBlock.h"
#include "boost/archive/text_oarchive.hpp"
#include "boost/archive/text_iarchive.hpp"
#include "boost/serialization/split_member.hpp"

class DataFileIndexer
	{
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
			
			// I hate myself for this
			char *uri_temp = new char[uri.length() + 1];
			strncpy(uri_temp, uri.c_str(), uri.length() + 1);
			session = scarab_session_connect(uri_temp);
			delete [] uri_temp;
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
		virtual ~DataFileIndexer();
		std::vector<ScarabDatumWrapper> events(const std::vector<unsigned int> &event_codes,
											   const MonkeyWorksTime lower_bound = MIN_MONKEY_WORKS_TIME(), 
											   const MonkeyWorksTime upper_bound = MAX_MONKEY_WORKS_TIME()) const;
		unsigned int getNEvents() const;
		
	};

#pragma GCC visibility pop
#endif

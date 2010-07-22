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
#include <MWorksCore/Utilities.h>
using namespace mw;
#include "EventWrapper.h"
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
			
            // Ben is a fucker.
			// Ben says: I hate myself for this
            // Dave says: you should, because you are a total fucker
            //char *uri_temp = new char[uri.length() + 1];
//			strncpy(uri_temp, uri.c_str(), uri.length() + 1);
//			session = scarab_session_connect(uri_temp);
//			delete [] uri_temp;
            
            // OK, so the above code is obviously insanely stupid and lazy
            // so, instead we need to contruct a meaningful uri from the 
            // readily available data_file field in the constructor 
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
        
        // DDC added to patch Ben's incompetance
        void reconstituteScarabSession(const boost::filesystem::path &data_file){
            boost::filesystem::path true_mwk_path = data_file / data_file.leaf();
            uri = std::string("ldobinary:file_readonly://") + true_mwk_path.string();
            
            char *uri_temp = new char[uri.length() + 1];
            strncpy(uri_temp, uri.c_str(), uri.length() + 1);
            session = scarab_session_connect(uri_temp);
            delete [] uri_temp;
        }
        
		std::vector<EventWrapper> events(const std::vector<unsigned int> &event_codes,
											   const MWorksTime lower_bound = MIN_MONKEY_WORKS_TIME(), 
											   const MWorksTime upper_bound = MAX_MONKEY_WORKS_TIME()) const;
		unsigned int getNEvents() const;
		MWorksTime getMinimumTime() const;
        MWorksTime getMaximumTime() const;
        
	};

#pragma GCC visibility pop
#endif

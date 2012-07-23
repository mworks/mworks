/*
 *  dfindex.h
 *  DataFileIndexer
 *
 *  Created by bkennedy on 2/20/08.
 *  Copyright 2008 MIT. All rights reserved.
 *
 */

#ifndef dfindex_
#define dfindex_

#include "DataFileIndexer.h"
#include "boost/filesystem/path.hpp"
#include "boost/archive/text_oarchive.hpp"
#include "boost/archive/text_iarchive.hpp"
#include "EventWrapper.h"
#include <MWorksCore/Utilities.h>
using namespace mw;

class dfindex
	{
	private:
		DataFileIndexer dfi;
		boost::filesystem::path mwk_data_file;

		boost::filesystem::path indexFile() const;
		void save() const;
		void load();
	public:
		dfindex(const boost::filesystem::path &data_file);

        std::string getFilePath(){
            return mwk_data_file.string();
        }
        
        MWorksTime getMinimumTime() const {
            return dfi.getMinimumTime();
        }
        
        MWorksTime getMaximumTime() const {
            return dfi.getMaximumTime();
        }
        
		std::vector<EventWrapper> events(const std::vector<unsigned int> &event_codes,
											   const MWorksTime lower_bound = MIN_MONKEY_WORKS_TIME(), 
											   const MWorksTime upper_bound = MAX_MONKEY_WORKS_TIME()) const;
	};


#endif //dfindex_



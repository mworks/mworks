/*
 *  dfindex.cpp
 *  DataFileIndexer
 *
 *  Created by bkennedy on 2/20/08.
 *  Copyright 2008 MIT. All rights reserved.
 *
 */

#include "dfindex.h"

#include <fstream>
#include <iostream>

#include <boost/filesystem/operations.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>


BEGIN_NAMESPACE_MW
BEGIN_NAMESPACE(scarab)


dfindex::dfindex(const boost::filesystem::path &data_file) :
    mwk_data_file(data_file),
    actual_mwk_file(prepareDataFile(mwk_data_file)),
    index_file(actual_mwk_file.string() + ".idx"),
    dfi(actual_mwk_file)
{
    if (boost::filesystem::exists(index_file)) {
        // Load the previously-created index
        std::ifstream ifs(index_file.string().c_str(), std::ios::binary);
        boost::archive::text_iarchive ia(ifs);
        ia >> dfi;
    } else {
        // Build and store a new index
        dfi.buildIndex(5000, 4);
        std::ofstream ofs(index_file.string().c_str(), std::ios::binary);
        boost::archive::text_oarchive oa(ofs);
        oa << dfi;
    }
}


std::size_t dfindex::getNumEvents() {
    return dfi.getNEvents();
}


MWTime dfindex::getTimeMin() {
    return dfi.getMinimumTime();
}


MWTime dfindex::getTimeMax() {
    return dfi.getMaximumTime();
}


void dfindex::selectEvents(const std::unordered_set<int> &event_codes, MWTime lower_bound, MWTime upper_bound) {
    std::set<unsigned int> codes;
    for (auto code : event_codes) {
        if (code < 0) {
            throw DataFileIndexerError(boost::format("Invalid event code: %d") % code);
        }
        codes.insert(code);
    }
    eventsIterator.reset(new DataFileIndexer::EventsIterator(dfi.getEventsIterator(codes, lower_bound, upper_bound)));
}


bool dfindex::nextEvent(int &code, MWTime &time, Datum &data) {
    if (!eventsIterator) {
        return false;
    }
    return eventsIterator->getNextEvent(code, time, data);
}


boost::filesystem::path dfindex::prepareDataFile(const boost::filesystem::path &mwk_data_file) {
	if (!boost::filesystem::exists(mwk_data_file)) {
		throw DataFileIndexerError(boost::format("File \"%s\" does not exist") % mwk_data_file.string());
	}
    
    const boost::filesystem::path actual_mwk_file(mwk_data_file / mwk_data_file.leaf());
	
	if (!boost::filesystem::is_directory(mwk_data_file)) {
		const boost::filesystem::path temp_location(mwk_data_file.string() + ".orig");
		boost::filesystem::rename(mwk_data_file, temp_location);
		boost::filesystem::create_directory(mwk_data_file);
		boost::filesystem::rename(temp_location, actual_mwk_file);
	}
	
	if (!boost::filesystem::exists(actual_mwk_file)) {
		throw DataFileIndexerError(boost::format("File \"%s\" does not exist") % actual_mwk_file.string());
	}
    
    return actual_mwk_file;
}


END_NAMESPACE(scarab)
END_NAMESPACE_MW

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



























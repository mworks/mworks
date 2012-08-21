/*
 *  dfindex.cpp
 *  DataFileIndexer
 *
 *  Created by bkennedy on 2/20/08.
 *  Copyright 2008 MIT. All rights reserved.
 *
 */

#include "dfindex.h"
#include "boost/filesystem/operations.hpp"
#include "boost/archive/tmpdir.hpp"
#include <exception>
#include <fstream>
#include <iostream>



dfindex::dfindex(const boost::filesystem::path &data_file) : mwk_data_file(data_file) {
	if(!boost::filesystem::exists(mwk_data_file)) {
		std::cerr << ".mwk file: " << mwk_data_file.string() << " doesn't exist" << std::endl;
		throw new std::exception;
	}
	
	if(!boost::filesystem::is_directory(mwk_data_file)) {
		std::string temp_location_string(mwk_data_file.string() + ".orig");
		boost::filesystem::path temp_location(temp_location_string);
		boost::filesystem::rename(mwk_data_file, temp_location);
		
		boost::filesystem::create_directory(mwk_data_file);
		boost::filesystem::rename(temp_location, mwk_data_file / mwk_data_file.leaf());
	}
	
	boost::filesystem::path actual_mwk_file(mwk_data_file / mwk_data_file.leaf());
	
	if(!boost::filesystem::exists(actual_mwk_file)) {
		std::cerr << ".mwk file: " << actual_mwk_file.string() << " doesn't exist" << std::endl;
		throw new std::exception;
	} else {			
		boost::filesystem::path index_file(this->indexFile());
		if(!boost::filesystem::exists(index_file)) {
			dfi = DataFileIndexer(actual_mwk_file, 5000, 4);	
			this->save();
		} 
		this->load();
	}
    
    //mwk_data_file = actual_mwk_file;
}

void dfindex::save() const {
	boost::filesystem::path index_file(this->indexFile());
	
	std::ofstream ofs(index_file.string().c_str());
	
	boost::archive::text_oarchive oa(ofs);
	oa << dfi;
}

void dfindex::load() {
	boost::filesystem::path index_file(this->indexFile());
	
	std::ifstream ifs(index_file.string().c_str(), std::ios::binary);
	boost::archive::text_iarchive ia(ifs);
	ia >> dfi;
    
    dfi.reconstituteScarabSession(mwk_data_file);
}

boost::filesystem::path dfindex::indexFile() const {
	return mwk_data_file / (mwk_data_file.leaf().string() + ".idx");
}





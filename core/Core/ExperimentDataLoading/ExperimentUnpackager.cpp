/*
 *  ExperimentUnpackager.cpp
 *  MWorksCore
 *
 *  Created by Ben Kennedy on 1/5/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "ExperimentUnpackager.h"
#include "ExperimentPackager.h"
#include "Scarab/scarab.h"
#include "Utilities.h"
#include "LoadingUtilities.h"
#include "SystemEventFactory.h"
#include <iostream>
#include <fstream>
#include "PlatformDependentServices.h"
#include "boost/filesystem/path.hpp"
#include "boost/filesystem/convenience.hpp"


BEGIN_NAMESPACE_MW


boost::filesystem::path 
ExperimentUnpackager::prependExperimentInstallPath(
													const std::string expname, 
													const std::string experimentFilename) {
	namespace bf = boost::filesystem;
	
    if(expname.empty() || experimentFilename.empty()) { 
		return bf::path(""); 
	}
	
    bf::path exp_tmp_path = getLocalExperimentStorageDir(expname);
    bf::path exp_path = exp_tmp_path /  bf::path(experimentFilename);
    
    bf::create_directories(exp_tmp_path);
    
    return exp_path;
}


bool ExperimentUnpackager::unpackageExperiment(Datum payload) {
	
	namespace bf = boost::filesystem;
	
    if(payload.getDataType() != M_DICTIONARY) {
        merror(M_NETWORK_MESSAGE_DOMAIN,
			   "Invalid payload type for experiment package");
        return false;
    }
	
 Datum experimentFilePackage = 
							payload.getElement(M_PACKAGER_EXPERIMENT_STRING);

	if(experimentFilePackage.getNElements() != 
							M_EXPERIMENT_PACKAGE_NUMBER_ELEMENTS_PER_UNIT || 
	   experimentFilePackage.getDataType() != M_DICTIONARY) 
		return false;
	
 Datum experimentFileName = 
				experimentFilePackage.getElement(M_PACKAGER_FILENAME_STRING);
	if(experimentFileName.getDataType() != M_STRING || 
	   experimentFileName.getStringLength() <= 0) 
		return false;
	
	bf::path experimentName(experimentFileName.getString());
	
	loadedExperimentFilename = prependExperimentInstallPath(removeFileExtension(experimentName.string()),
								experimentName.string());
	
	bf::path experimentPath = loadedExperimentFilename.branch_path();
	
	createExperimentInstallDirectoryStructure(experimentName.string());
	
	
	// create the XML file
 Datum experimentFileBuffer = 
		experimentFilePackage.getElement(M_PACKAGER_CONTENTS_STRING);
	
	if(experimentFileBuffer.getDataType() != M_STRING ||
	   experimentFileBuffer.getStringLength() <= 0) 
		return false;
	
	if(!(createFile(Datum(loadedExperimentFilename.string().c_str()), 
					experimentFileBuffer))) {
        // failed to create experiment file
        merror(M_FILE_MESSAGE_DOMAIN,
			   "Failed to create server side experiment file %s", 
			   loadedExperimentFilename.string().c_str());
        return false;
    }
	
	// create all of the other media files
 Datum mediaFileList = payload.getElement(M_PACKAGER_MEDIA_BUFFERS_STRING);
	
	if(mediaFileList.isList()) {
	
		for(int i=0; i<mediaFileList.getNElements(); ++i) {
		 Datum mediaFilePackage = mediaFileList.getElement(i);
		
			if(mediaFilePackage.getDataType() != M_DICTIONARY |
			   mediaFilePackage.getNElements() != 2) {
				merror(M_FILE_MESSAGE_DOMAIN,
					   "incorrectly packaged media files");
				return false;
			}
		
		 Datum mediaFileName = 
						mediaFilePackage.getElement(M_PACKAGER_FILENAME_STRING);
		 Datum mediaFileBuffer = 
						mediaFilePackage.getElement(M_PACKAGER_CONTENTS_STRING);
		
			if(mediaFileName.getDataType() != M_STRING ||
			   mediaFileName.getStringLength() <= 0 ||
			   mediaFileBuffer.getDataType() != M_STRING) return false;
		
			std::string filename(mediaFileName.getString());
			std::string filenameWPath = experimentPath.string() + "/" + filename;
		
			if(!(createFile(Datum(filenameWPath.c_str()), 
							mediaFileBuffer))) {
				// failed to create experiment file
				merror(M_FILE_MESSAGE_DOMAIN,
					   "Failed to create server side experiment file %s", 
					   filenameWPath.c_str());
				return false;
			}
		}
	}
	
	expandRangeReplicatorItems(loadedExperimentFilename);
	modifyExperimentMediaPaths(loadedExperimentFilename);
	
    return true;
}

boost::filesystem::path 
ExperimentUnpackager::getUnpackagedExperimentPath() {
	return loadedExperimentFilename;
	
}


bool ExperimentUnpackager::createFile(Datum filename, Datum buffer) {	
	if(buffer.getDataType() != M_STRING ||
	   filename.getDataType() != M_STRING) return false;
    
    boost::filesystem::path filePath(filename.getString());
    boost::filesystem::create_directories(filePath.parent_path());
	
    // create an output file.
    std::ofstream outFile(filePath.string().c_str()); //, ios::binary);
											//write the data
    outFile.write(buffer.getString(), buffer.getStringLength());
    // flush buffer
    outFile.flush();
    // close the handle
    outFile.close();
    return true;
}


END_NAMESPACE_MW

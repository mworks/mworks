/*
 *  ExperimentUnpackager.cpp
 *  MonkeyWorksCore
 *
 *  Created by Ben Kennedy on 1/5/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "ExperimentUnpackager.h"
#include "ExperimentPackager.h"
#include "EmbeddedPerlInterpreter.h"
#include "Scarab/scarab.h"
#include "Utilities.h"
#include "LoadingUtilities.h"
#include "EventFactory.h"
#include <iostream>
#include <fstream>
#include "PlatformDependentServices.h"
#include "boost/filesystem/path.hpp"
using namespace mw;

boost::filesystem::path 
ExperimentUnpackager::prependExperimentInstallPath(
													const std::string expname, 
													const std::string experimentFilename) {
	namespace bf = boost::filesystem;
	
    if(expname.empty() || experimentFilename.empty()) { 
		return bf::path("", bf::native); 
	}
	bf::path ei(experimentInstallPath());
	bf::path es(experimentStorageDirectoryName());
	
	return ei / bf::path(expname, bf::native) / es / bf::path(experimentFilename, bf::native);
}


bool ExperimentUnpackager::unpackageExperiment(Data payload) {
	
	namespace bf = boost::filesystem;
	
    if(payload.getDataType() != M_DICTIONARY) {
        merror(M_NETWORK_MESSAGE_DOMAIN,
			   "Invalid payload type for experiment package");
        return false;
    }
	
	Data experimentFilePackage = 
							payload.getElement(M_PACKAGER_EXPERIMENT_STRING);

	if(experimentFilePackage.getNElements() != 
							M_EXPERIMENT_PACKAGE_NUMBER_ELEMENTS_PER_UNIT || 
	   experimentFilePackage.getDataType() != M_DICTIONARY) 
		return false;
	
	Data experimentFileName = 
				experimentFilePackage.getElement(M_PACKAGER_FILENAME_STRING);
	if(experimentFileName.getDataType() != M_STRING || 
	   experimentFileName.getStringLength() <= 0) 
		return false;
	
	bf::path experimentName(experimentFileName.getString(), bf::native);
	
	loadedExperimentFilename = prependExperimentInstallPath(removeFileExtension(experimentName.string()),
								experimentName.string());
	
	bf::path experimentPath = loadedExperimentFilename.branch_path();
	
	createExperimentInstallDirectoryStructure(experimentName.string());
	
	
	// create the XML file
	Data experimentFileBuffer = 
		experimentFilePackage.getElement(M_PACKAGER_CONTENTS_STRING);
	
	if(experimentFileBuffer.getDataType() != M_STRING ||
	   experimentFileBuffer.getStringLength() <= 0) 
		return false;
	
	if(!(createFile(Data(loadedExperimentFilename.string().c_str()), 
					experimentFileBuffer))) {
        // failed to create experiment file
        merror(M_FILE_MESSAGE_DOMAIN,
			   "Failed to create server side experiment file %s", 
			   loadedExperimentFilename.string().c_str());
        return false;
    }
	
	// create all of the other media files
	Data mediaFileList = payload.getElement(M_PACKAGER_MEDIA_BUFFERS_STRING);
	
	if(mediaFileList.isList()) {
	
		for(int i=0; i<mediaFileList.getNElements(); ++i) {
			Data mediaFilePackage = mediaFileList.getElement(i);
		
			if(mediaFilePackage.getDataType() != M_DICTIONARY |
			   mediaFilePackage.getNElements() != 2) {
				merror(M_FILE_MESSAGE_DOMAIN,
					   "incorrectly packaged media files");
				return false;
			}
		
			Data mediaFileName = 
						mediaFilePackage.getElement(M_PACKAGER_FILENAME_STRING);
			Data mediaFileBuffer = 
						mediaFilePackage.getElement(M_PACKAGER_CONTENTS_STRING);
		
			if(mediaFileName.getDataType() != M_STRING ||
			   mediaFileName.getStringLength() <= 0 ||
			   mediaFileBuffer.getDataType() != M_STRING) return false;
		
			std::string filename(mediaFileName.getString());
			std::string filenameWPath = experimentPath.string() + "/" + filename;
		
			if(!(createFile(Data(filenameWPath.c_str()), 
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


bool ExperimentUnpackager::createFile(Data filename, Data buffer) {	
	if(buffer.getDataType() != M_STRING ||
	   filename.getDataType() != M_STRING) return false;
	
    // create an output file.
    ofstream outFile(filename.getString()); //, ios::binary);
											//write the data
    outFile.write(buffer.getString(), buffer.getStringLength());
    // flush buffer
    outFile.flush();
    // close the handle
    outFile.close();
    return true;
}

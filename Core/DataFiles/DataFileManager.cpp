/**
 * DataFileManager.cpp
 *
 * History:
 * David Cox on Wed Oct 06 2004 - Created.
 * Paul Jankunas on 09/06/05 - Added initialzation code for filename
 *  and copying of filename to the open file routine.
 *
 * Copyright (c) 2004 MIT. All rights reserved.
 */

#include "Experiment.h"
#include "Event.h"
#include "DataFileManager.h"
#include "Utilities.h"
#include "ScarabServices.h"
#include <string>
#include "EventFactory.h"
#include "EventBuffer.h"
#include "PlatformDependentServices.h"
using namespace mw;

namespace mw {
	DataFileManager *GlobalDataFileManager;
}

DataFileManager::DataFileManager() {
	
	
    scarab_connection = shared_ptr<ScarabWriteConnection>();
    //session = NULL;
    file_open = false;
	
}

DataFileManager::~DataFileManager() { }

int DataFileManager::openFile(const Data &oeDatum) {
	std::string dFile(oeDatum.getElement(DATA_FILE_FILENAME).getString());
	DataFileOptions opt = (DataFileOptions)oeDatum.getElement(DATA_FILE_OPTIONS).getInteger();
	
	if(dFile.size() == 0) {
		merror(M_FILE_MESSAGE_DOMAIN, 
			   "Attempt to open an empty data file");
		return -1;
	}
	if(GlobalDataFileManager->isFileOpen()) {
		mwarning(M_FILE_MESSAGE_DOMAIN,
				 "Data file already open at %s", 
				 (GlobalDataFileManager->getFilename()).c_str());
		return -1;
	}
	
	return openFile(dFile, opt);
}

int DataFileManager::openFile(std::string _filename, DataFileOptions opt) {
    // first we need to format the file name with the correct path and
    // extension
	std::string _ext_(appendDataFileExtension(
											  prependDataFilePath(_filename.c_str()).string()));    
    filename = _ext_;
    
    // form the scarab uri
    std::string prefix = "ldobinary:file://";
    std::string uri = prefix + filename;
	
	if(opt == M_NO_OVERWRITE) {
		FILE *fd = fopen(filename.c_str(), "r");
		if(fd != 0) {
			// badness....don't overwrite my file!
			fclose(fd);
			
			merror(M_FILE_MESSAGE_DOMAIN,
				   "Can't overwrite existing file: %s", filename.c_str());
			//GlobalSystemEventVariable->setValue(EventFactory::dataFileOpenedResponse(filename.c_str(), M_COMMAND_FAILURE));
			GlobalBufferManager->putEvent(EventFactory::dataFileOpenedResponse(filename.c_str(), 
																				M_COMMAND_FAILURE));
			return -1;
		}
	}			
	
    if(scarab_create_file(filename.c_str()) != 0){
		merror(M_FILE_MESSAGE_DOMAIN,
			   "Could not create file: %s", filename.c_str());
		return -1;
	}
    scarab_connection = shared_ptr<ScarabWriteConnection>(new ScarabWriteConnection(GlobalBufferManager, uri));
    scarab_connection->connect();
	
    if(scarab_connection->isConnected()) {
        scarab_connection->startThread(M_DATAFILE_SERVICE_INTERVAL_US);
        file_open = true;
		
        // write out the event-code to name/description mapping
        // this is an essential part of the self-describing nature of the
        // MonkeyWorks/Scarab format
		GlobalBufferManager->putEvent(EventFactory::componentCodecPackage());
		GlobalBufferManager->putEvent(EventFactory::codecPackage());
		GlobalBufferManager->putEvent(EventFactory::currentExperimentState());
		GlobalVariableRegistry->announceAll();
    } else {
        merror(M_FILE_MESSAGE_DOMAIN,
			   "Failed to open file: %s", uri.c_str());
        GlobalBufferManager->putEvent(EventFactory::dataFileOpenedResponse(filename.c_str(), 
																			M_COMMAND_FAILURE));
        return -1;
    }
    
	mprintf(M_FILE_MESSAGE_DOMAIN, "Opening data file: %s", filename.c_str());
	
    // everything went ok so issue the success event
    GlobalBufferManager->putEvent(EventFactory::dataFileOpenedResponse(filename.c_str(), 
																		M_COMMAND_SUCCESS)); 
    return 0;
}

int DataFileManager::closeFile() {
    if(file_open) {
        scarab_connection->disconnect();
        // delete scarab_connection;
        file_open = false;
		
		mprintf(M_FILE_MESSAGE_DOMAIN, "Closing data file: %s", filename.c_str());
        GlobalBufferManager->putEvent(EventFactory::dataFileClosedResponse(filename.c_str(), 
																			M_COMMAND_SUCCESS)); 
    } else {
		merror(M_FILE_MESSAGE_DOMAIN,
			   "Attempt to close a data file when there isn't one open");		
	}
    return 0;
}

bool DataFileManager::isFileOpen() {
    return file_open;
}

std::string DataFileManager::getFilename() { 
    return filename;
}


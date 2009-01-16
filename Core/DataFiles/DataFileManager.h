/**
 * DataFileManager.h
 *
 * Discussion: This file contains rough, beginning sketches of what the
 * basic Data File support should look like. "Data File" in this context 
 * refers to the files to which realtime events (Event objects) are streamed.
 * Currently, each DataFileManager manages just one file, so if you wanted 
 * to stream to multiple files at once (e.g. if all of the eye samples were 
 * supposed to go one place, and everything else somewhere else), then you'd
 * need to create multiple DataFileManagers (better event-routing would 
 * be required as well to make one type of events go to a particular file).
 *
 * History:
 * David Cox on Wed Oct 06 2004 - Created
 * Paul Jankunas on 09/06/05 - Added destructor because filename is now copied
 *
 * Copyright (c) 2004 MIT. All rights reserved.
 */

#ifndef DATA_FILE_MANAGER_H__
#define DATA_FILE_MANAGER_H__

#include "ScarabServices.h"
#include "ScarabWriteConnection.h"
#include "Event.h"
#include "EventFactory.h"

#define DATA_FILE_FILENAME	"file"
#define DATA_FILE_OPTIONS	"options"

#define M_DATAFILE_SERVICE_INTERVAL_US	20000
namespace mw {
	class DataFileManager{
		
	private:
		//mEventBufferReader *buffer_reader;
		
		std::string filename;
		bool file_open;
		
		shared_ptr <ScarabWriteConnection> scarab_connection;
		
		//ScarabSession *session;
		//ScarabDatum *event_template;
		
	public:
		
		DataFileManager();
        ~DataFileManager();
        
        /*!
         * @function openFile
         * @discussion TODO.... issues a M_DATA_FILE_OPENED event
         */
		int openFile(const Data &openFileDatum);
		int openFile(std::string filename, DataFileOptions opt);
		
        /*!
         * @function closeFile
         * @discussion TODO.... issues a M_DATA_FILE_CLOSED event
         */
		int closeFile();
		
		bool isFileOpen();
		std::string getFilename();
		
		int serviceBuffer();
		int serviceBufferPeriodically();
		
		// Write out a dictionary that maps event codes onto event names
		// this should minimally be done at the head the file, but there might 
        //also arise cases where
		// you might want to write this later as well (e.g. to better ensure
        // data integrity, or if
		// more events were somehow added on the fly, etc. etc.)
		// void writeCodec();
	};
	
	
	extern DataFileManager *GlobalDataFileManager;
}
#endif
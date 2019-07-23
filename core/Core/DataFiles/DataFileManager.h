/**
 * DataFileManager.h
 *
 * Discussion: This file contains rough, beginning sketches of what the
 * basic Datum File support should look like. "Data File" in this context 
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

#include <thread>

#include "MWK2File.hpp"
#include "RegisteredSingleton.h"
#include "SystemEventFactory.h"

#define DATA_FILE_FILENAME	"file"
#define DATA_FILE_OVERWRITE	"overwrite"


BEGIN_NAMESPACE_MW


class DataFileManager {
    
public:
    REGISTERED_SINGLETON_CODE_INJECTION(DataFileManager)
    
    DataFileManager();
    ~DataFileManager();
    
    /*!
     * @function openFile
     * @discussion TODO.... issues a M_DATA_FILE_OPENED event
     */
    bool openFile(const Datum &openFileDatum);
    bool openFile(const std::string &filename, bool overwrite);
    
    /*!
     * @function closeFile
     * @discussion TODO.... issues a M_DATA_FILE_CLOSED event
     */
    bool closeFile();
    
    bool isFileOpen() const { return eventHandlerThread.joinable(); }
    const std::string& getFilename() const { return filename; }
    
private:
    void handleEvents();
    
    std::string filename;
    
    std::unique_ptr<MWK2Writer> mwk2Writer;
    std::unordered_set<int> excludedEventCodes;
    std::unique_ptr<EventBufferReader> eventBufferReader;
    std::thread eventHandlerThread;
    static_assert(ATOMIC_BOOL_LOCK_FREE == 2, "std::atomic_bool is not always lock-free");
    std::atomic_bool running;
    
};


END_NAMESPACE_MW


#endif

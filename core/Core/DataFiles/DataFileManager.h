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


BEGIN_NAMESPACE_MW


class DataFileManager {
    
    class DataFile {
    public:
        static bool create(const std::string &filename, bool overwrite, std::unique_ptr<DataFile> &dataFile);
        ~DataFile();
        const std::string & getFilename() const { return filename; }
        
    private:
        explicit DataFile(const std::string &filename);
        void handleEvents();
        
        const std::string filename;
        const std::unordered_set<int> excludedEventCodes;
        MWK2Writer mwk2Writer;
        EventBufferReader eventBufferReader;
        static_assert(ATOMIC_BOOL_LOCK_FREE == 2, "std::atomic_bool is not always lock-free");
        std::atomic_bool running;
        std::thread eventHandlerThread;
    };
    
public:
    REGISTERED_SINGLETON_CODE_INJECTION(DataFileManager)
    
    bool openFile(std::string filename, bool overwrite);
    void closeFile();
    std::string getFilename() const;
    
private:
    std::unique_ptr<DataFile> dataFile;
    
    using lock_guard = std::lock_guard<std::mutex>;
    mutable lock_guard::mutex_type mutex;
    
};


END_NAMESPACE_MW


#endif

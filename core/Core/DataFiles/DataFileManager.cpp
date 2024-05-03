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

#include "DataFileManager.h"

#include <boost/filesystem/operations.hpp>

#include "PlatformDependentServices.h"


BEGIN_NAMESPACE_MW


bool DataFileManager::DataFile::create(const std::string &filename,
                                       bool overwrite,
                                       std::unique_ptr<DataFile> &dataFile)
{
    const auto filepath = boost::filesystem::path(filename);
    
    // Check for an existing file
    if (boost::filesystem::exists(filepath)) {
        if (!overwrite) {
            merror(M_FILE_MESSAGE_DOMAIN, "Can't overwrite existing file \"%s\"", filename.c_str());
            return false;
        }
        try {
            boost::filesystem::remove(filepath);
        } catch (const std::exception &e) {
            merror(M_FILE_MESSAGE_DOMAIN, "Can't remove existing file \"%s\": %s", filename.c_str(), e.what());
            return false;
        }
    }
    
    // Ensure that the destination directory exists
    try {
        boost::filesystem::create_directories(filepath.parent_path());
    } catch (const std::exception &e) {
        merror(M_FILE_MESSAGE_DOMAIN, "Could not create data file destination directory: %s", e.what());
        return false;
    }
    
    // Create the file
    try {
        dataFile.reset(new DataFile(filename));
    } catch (const SimpleException &e) {
        merror(M_FILE_MESSAGE_DOMAIN, "Could not create file \"%s\": %s", filename.c_str(), e.what());
        return false;
    }
    
    return true;
}


DataFileManager::DataFile::~DataFile() {
    if (eventHandlerThread.joinable()) {
        running = false;
        eventHandlerThread.join();
    }
}


static std::unordered_set<int> getExcludedEventCodes() {
    std::unordered_set<int> excludedEventCodes;
    for (auto &var : global_variable_registry->getGlobalVariables()) {
        if (var->getProperties().getExcludeFromDataFile()) {
            excludedEventCodes.insert(var->getCodecCode());
        }
    }
    return excludedEventCodes;
}


DataFileManager::DataFile::DataFile(const std::string &filename) :
    filename(filename),
    excludedEventCodes(getExcludedEventCodes()),
    mwk2Writer(filename),
    eventBufferReader(global_outgoing_event_buffer),
    running(true),
    eventHandlerThread([this]() { handleEvents(); })
{ }


void DataFileManager::DataFile::handleEvents() {
    constexpr MWTime nextEventTimeout = 500 * 1000;  // 500ms
    constexpr std::size_t maxPendingEvents = 1000;
    
    std::vector<boost::shared_ptr<Event>> pendingEvents;
    
    while (running) {
        // Wait for next event or timeout
        auto event = eventBufferReader.getNextEvent(nextEventTimeout);
        
        // Process all currently-available events
        while (event) {
            // Collect all currently-available events (up to maxPendingEvents)
            do {
                if (!excludedEventCodes.count(event->getEventCode())) {
                    pendingEvents.push_back(event);
                }
                event = eventBufferReader.getNextEvent();
            } while (event && (pendingEvents.size() < maxPendingEvents));
            
            // Write pending events
            if (!pendingEvents.empty()) {
                try {
                    mwk2Writer.writeEvents(pendingEvents);
                } catch (const SimpleException &e) {
                    merror(M_FILE_MESSAGE_DOMAIN,
                           "Failed to write %lu events to data file: %s",
                           pendingEvents.size(),
                           e.what());
                }
                pendingEvents.clear();
            }
        }
    }
    
    // Write termination event
    try {
        mwk2Writer.writeEvent(boost::make_shared<Event>(RESERVED_TERMINATION_CODE, Datum()));
    } catch (const SimpleException &e) {
        merror(M_FILE_MESSAGE_DOMAIN, "Failed to write termination event to data file: %s", e.what());
    }
}


SINGLETON_INSTANCE_STATIC_DECLARATION(DataFileManager)


static inline std::string getFullFilename(const std::string &filename) {
    return appendDataFileExtension(prependDataFilePath(filename).string());
}


bool DataFileManager::openFile(const std::string &filename, bool overwrite) {
    lock_guard lock(mutex);
    if (dataFile) {
        mwarning(M_FILE_MESSAGE_DOMAIN, "Data file already open at \"%s\"", dataFile->getFilename().c_str());
        return true;
    }
    return openFile(getFullFilename(filename), overwrite, dataFile);
}


bool DataFileManager::openFile(const std::string &filename, bool overwrite, std::unique_ptr<DataFile> &dataFile) {
    if (!DataFile::create(filename, overwrite, dataFile)) {
        global_outgoing_event_buffer->putEvent(SystemEventFactory::dataFileOpenedResponse(filename, M_COMMAND_FAILURE));
        return false;
    }
    
    // Announce component codec, variable codec, experiment state, and all current
    // variable values
    global_outgoing_event_buffer->putEvent(SystemEventFactory::componentCodecPackage());
    global_outgoing_event_buffer->putEvent(SystemEventFactory::codecPackage());
    global_outgoing_event_buffer->putEvent(SystemEventFactory::currentExperimentState());
    global_variable_registry->announceAll();
    
    mprintf(M_FILE_MESSAGE_DOMAIN, "Opened data file: %s", filename.c_str());
    
    // everything went ok so issue the success event
    global_outgoing_event_buffer->putEvent(SystemEventFactory::dataFileOpenedResponse(filename, M_COMMAND_SUCCESS));
    return true;
}


void DataFileManager::closeFile() {
    lock_guard lock(mutex);
    if (dataFile) {
        closeFile(dataFile);
    }
}


void DataFileManager::closeFile(std::unique_ptr<DataFile> &dataFile) {
    mprintf(M_FILE_MESSAGE_DOMAIN, "Closing data file...");
    auto filename = dataFile->getFilename();  // Make a copy to use after dataFile is destroyed
    dataFile.reset();
    mprintf(M_FILE_MESSAGE_DOMAIN, "Closed data file: %s", filename.c_str());
    global_outgoing_event_buffer->putEvent(SystemEventFactory::dataFileClosedResponse(filename, M_COMMAND_SUCCESS));
}


std::string DataFileManager::getFilename() const {
    lock_guard lock(mutex);
    std::string filename;
    if (dataFile) {
        filename = dataFile->getFilename();
    }
    return filename;
}


void DataFileManager::setAutoOpenFilename(const boost::shared_ptr<Variable> &filename) {
    lock_guard lock(mutex);
    if (autoOpenFilename) {
        throw SimpleException(M_FILE_MESSAGE_DOMAIN, "Automatic data-file management is already configured");
    }
    autoOpenFilename = filename;
}


void DataFileManager::clearAutoOpenFilename() {
    lock_guard lock(mutex);
    autoOpenFilename.reset();
}


bool DataFileManager::canAutoOpenFile() const {
    lock_guard lock(mutex);
    return bool(autoOpenFilename);
}


bool DataFileManager::autoOpenFile() {
    lock_guard lock(mutex);
    
    if (!autoOpenFilename) {
        // Nothing to do
        return true;
    }
    
    // Evaluate the filename
    std::string filename;
    try {
        filename = autoOpenFilename->getValue().getString();
    } catch (const SimpleException &e) {
        merror(M_FILE_MESSAGE_DOMAIN, "Filename evaluation failed: %s", e.what());
        return false;
    }
    if (filename.empty()) {
        merror(M_FILE_MESSAGE_DOMAIN, "Evaluated filename is empty");
        return false;
    }
    filename = getFullFilename(filename);
    
    if (dataFile) {
        if (dataFile->getFilename() == filename) {
            // The desired file is already open.  Nothing else to do.
            return true;
        }
        // Close the current file
        closeFile(dataFile);
    }
    
    return openFile(filename, false, dataFile);  // Never overwrite
}


END_NAMESPACE_MW

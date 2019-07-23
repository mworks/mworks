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

#include <boost/filesystem/convenience.hpp>

#include "PlatformDependentServices.h"


BEGIN_NAMESPACE_MW


SINGLETON_INSTANCE_STATIC_DECLARATION(DataFileManager)


DataFileManager::DataFileManager() :
    running(false)
{ }


DataFileManager::~DataFileManager() {
    if (eventHandlerThread.joinable()) {
        running = false;
        eventHandlerThread.join();
    }
}


bool DataFileManager::openFile(const Datum &oeDatum) {
    std::string dFile(oeDatum.getElement(DATA_FILE_FILENAME).getString());
    auto overwrite = oeDatum.getElement(DATA_FILE_OVERWRITE).getBool();
    
    if(dFile.size() == 0) {
        merror(M_FILE_MESSAGE_DOMAIN, "Attempt to open data file with an empty name");
        return false;
    }
    
    return openFile(dFile, overwrite);
}


bool DataFileManager::openFile(const std::string &_filename, bool overwrite) {
    if (isFileOpen()) {
        mwarning(M_FILE_MESSAGE_DOMAIN, "Data file already open at \"%s\"", filename.c_str());
        return false;
    }
    
    // first we need to format the file name with the correct path and
    // extension
    filename = appendDataFileExtension(prependDataFilePath(_filename).string());
    const auto filepath = boost::filesystem::path(filename);
    
    if (boost::filesystem::exists(filepath)) {
        if (!overwrite) {
            merror(M_FILE_MESSAGE_DOMAIN, "Can't overwrite existing file \"%s\"", filename.c_str());
            global_outgoing_event_buffer->putEvent(SystemEventFactory::dataFileOpenedResponse(filename.c_str(),
                                                                                              M_COMMAND_FAILURE));
            return false;
        }
        try {
            boost::filesystem::remove(filepath);
        } catch (const std::exception &e) {
            merror(M_FILE_MESSAGE_DOMAIN, "Can't remove existing file \"%s\": %s", filename.c_str(), e.what());
            global_outgoing_event_buffer->putEvent(SystemEventFactory::dataFileOpenedResponse(filename.c_str(),
                                                                                              M_COMMAND_FAILURE));
            return false;
        }
    }
    
    // Ensure that the destination directory exists
    try {
        boost::filesystem::create_directories(filepath.parent_path());
    } catch (const std::exception &e) {
        merror(M_FILE_MESSAGE_DOMAIN, "Could not create data file destination directory: %s", e.what());
        global_outgoing_event_buffer->putEvent(SystemEventFactory::dataFileOpenedResponse(filename.c_str(),
                                                                                          M_COMMAND_FAILURE));
        return false;
    }
    
    // Create the file
    try {
        mwk2Writer.reset(new MWK2Writer(filename));
    } catch (const SimpleException &e) {
        merror(M_FILE_MESSAGE_DOMAIN, "Could not create file \"%s\": %s", filename.c_str(), e.what());
        global_outgoing_event_buffer->putEvent(SystemEventFactory::dataFileOpenedResponse(filename.c_str(),
                                                                                          M_COMMAND_FAILURE));
        return false;
    }
    
    // Generate list of excluded event codes
    excludedEventCodes.clear();
    for (auto &var : global_variable_registry->getGlobalVariables()) {
        if (var->getProperties()->getExcludeFromDataFile()) {
            excludedEventCodes.insert(var->getCodecCode());
        }
    }
    
    // Create the event buffer reader
    eventBufferReader.reset(new EventBufferReader(global_outgoing_event_buffer));
    
    // Start the event handler thread
    running = true;
    eventHandlerThread = std::thread([this]() { handleEvents(); });
    
    // Announce component codec, variable codec, experiment state, and all current
    // variable values
    global_outgoing_event_buffer->putEvent(SystemEventFactory::componentCodecPackage());
    global_outgoing_event_buffer->putEvent(SystemEventFactory::codecPackage());
    global_outgoing_event_buffer->putEvent(SystemEventFactory::currentExperimentState());
    global_variable_registry->announceAll();
    
    mprintf(M_FILE_MESSAGE_DOMAIN, "Opened data file: %s", filename.c_str());
    
    // everything went ok so issue the success event
    global_outgoing_event_buffer->putEvent(SystemEventFactory::dataFileOpenedResponse(filename.c_str(),
                                                                                      M_COMMAND_SUCCESS));
    return true;
}


bool DataFileManager::closeFile() {
    if (!isFileOpen()) {
        merror(M_FILE_MESSAGE_DOMAIN, "Attempt to close a data file when there isn't one open");
    } else {
        mprintf(M_FILE_MESSAGE_DOMAIN, "Closing data file...");
        
        running = false;
        eventHandlerThread.join();
        eventBufferReader.reset();
        mwk2Writer.reset();
        
        mprintf(M_FILE_MESSAGE_DOMAIN, "Closed data file: %s", filename.c_str());
        global_outgoing_event_buffer->putEvent(SystemEventFactory::dataFileClosedResponse(filename.c_str(),
                                                                                          M_COMMAND_SUCCESS));
    }
    return true;
}


void DataFileManager::handleEvents() {
    constexpr MWTime nextEventTimeout = 500 * 1000;  // 500ms
    constexpr std::size_t maxPendingEvents = 1000;
    
    std::vector<boost::shared_ptr<Event>> pendingEvents;
    
    while (running) {
        // Wait for next event or timeout
        auto event = eventBufferReader->getNextEvent(nextEventTimeout);
        
        // Process all currently-available events
        while (event) {
            // Collect all currently-available events (up to maxPendingEvents)
            do {
                if (!excludedEventCodes.count(event->getEventCode())) {
                    pendingEvents.push_back(event);
                }
                event = eventBufferReader->getNextEvent();
            } while (event && (pendingEvents.size() < maxPendingEvents));
            
            // Write pending events
            if (!pendingEvents.empty()) {
                try {
                    mwk2Writer->writeEvents(pendingEvents);
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
        mwk2Writer->writeEvent(boost::make_shared<Event>(RESERVED_TERMINATION_CODE, Datum()));
    } catch (const SimpleException &e) {
        merror(M_FILE_MESSAGE_DOMAIN, "Failed to write termination event to data file: %s", e.what());
    }
}


END_NAMESPACE_MW

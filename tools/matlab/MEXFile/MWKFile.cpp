//
//  MWKFile.cpp
//  MEX File
//
//  Created by Christopher Stawarz on 2/7/23.
//  Copyright © 2023 MWorks Project. All rights reserved.
//

#include "MWKFile.hpp"

#include "Converters.h"

#define EVENT_CODE_FIELD_NAME "event_code"
#define EVENT_TIME_FIELD_NAME "time_us"
#define EVENT_DATA_FIELD_NAME "data"


BEGIN_NAMESPACE_MW_MATLAB


void MWKFile::open(MEXInputs &inputs, MEXOutputs &outputs) {
    boost::filesystem::path filename;
    Handle handle;
    
    inputs >> filename;
    
    try {
        auto reader = DataFileReader::openDataFile(filename);
        handle = reinterpret_cast<Handle>(reader.get());
        getReaderMap()[handle] = std::move(reader);
    } catch (const std::exception &e) {
        throwError(e.what());
    }
    
    outputs << handle;
}


void MWKFile::close(MEXInputs &inputs, MEXOutputs &outputs) {
    Handle handle;
    inputs >> handle;
    
    // If handle isn't in readerMap, this does nothing
    getReaderMap().erase(handle);
}


void MWKFile::getNumEvents(MEXInputs &inputs, MEXOutputs &outputs) {
    auto &reader = getReader(inputs);
    outputs << static_cast<mxUint64>(reader.getNumEvents());
}


void MWKFile::getMinTime(MEXInputs &inputs, MEXOutputs &outputs) {
    auto &reader = getReader(inputs);
    outputs << reader.getTimeMin();
}


void MWKFile::getMaxTime(MEXInputs &inputs, MEXOutputs &outputs) {
    auto &reader = getReader(inputs);
    outputs << reader.getTimeMax();
}


void MWKFile::selectEvents(MEXInputs &inputs, MEXOutputs &outputs) {
    auto &reader = getReader(inputs);
    
    std::unordered_set<int> codes;
    MWTime minTime, maxTime;
    inputs >> codes >> minTime >> maxTime;
    
    try {
        reader.selectEvents(codes, minTime, maxTime);
    } catch (const std::exception &e) {
        throwError(e.what());
    }
}


void MWKFile::nextEvent(MEXInputs &inputs, MEXOutputs &outputs) {
    auto &reader = getReader(inputs);
    
    ArrayPtr eventCode, eventTime, eventData;
    try {
        int code;
        MWTime time;
        Datum data;
        if (reader.nextEvent(code, time, data)) {
            eventCode = Array::createScalar(code);
            eventTime = Array::createScalar(time);
            eventData = convertDatumToArray(data);
        }
    } catch (const std::exception &e) {
        throwError(e.what());
    }
    
    if (outputs.count() == 3) {
        outputs << (eventCode ? std::move(eventCode) : Array::createEmpty<int>());
        outputs << (eventTime ? std::move(eventTime) : Array::createEmpty<MWTime>());
        outputs << (eventData ? std::move(eventData) : Array::createEmpty<double>());
    } else {
        outputs << Array::createStruct({ EVENT_CODE_FIELD_NAME, EVENT_TIME_FIELD_NAME, EVENT_DATA_FIELD_NAME },
                                       std::move(eventCode),
                                       std::move(eventTime),
                                       std::move(eventData));
    }
}


void MWKFile::getEvents(MEXInputs &inputs, MEXOutputs &outputs) {
    auto &reader = getReader(inputs);
    
    std::vector<int> eventCodes;
    std::vector<MWTime> eventTimes;
    std::vector<ArrayPtr> eventValues;
    try {
        int code;
        MWTime time;
        Datum value;
        while (reader.nextEvent(code, time, value)) {
            eventCodes.push_back(code);
            eventTimes.push_back(time);
            eventValues.emplace_back(convertDatumToArray(value));
        }
    } catch (const std::exception &e) {
        throwError(e.what());
    }
    
    if (outputs.count() == 3) {
        outputs << eventCodes << eventTimes << std::move(eventValues);
    } else {
        auto result = Array::createStruct(eventCodes.size(),
                                          { EVENT_CODE_FIELD_NAME, EVENT_TIME_FIELD_NAME, EVENT_DATA_FIELD_NAME });
        for (std::size_t i = 0; i < eventCodes.size(); i++) {
            mxSetFieldByNumber(result.get(), i, 0, Array::createScalar(eventCodes[i]).release());
            mxSetFieldByNumber(result.get(), i, 1, Array::createScalar(eventTimes[i]).release());
            mxSetFieldByNumber(result.get(), i, 2, eventValues[i].release());
        }
        outputs << std::move(result);
    }
}


inline void MWKFile::throwError(const std::string &msg) {
    throwMATLABError("MWorks:MWKFileError", msg);
}


auto MWKFile::getReaderMap() -> ReaderMap & {
    static ReaderMap readerMap;
    return readerMap;
}


DataFileReader & MWKFile::getReader(MEXInputs &inputs) {
    Handle handle;
    inputs >> handle;
    
    auto &readerMap = getReaderMap();
    auto iter = readerMap.find(handle);
    if (iter == readerMap.end()) {
        throwError("File is not open");
    }
    return *(iter->second);
}


END_NAMESPACE_MW_MATLAB

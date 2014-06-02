//
//  getEvents.cpp
//  MATLABTools
//
//  Created by Christopher Stawarz on 12/17/12.
//  Copyright (c) 2012 MWorks Project. All rights reserved.
//

#include "getEvents.h"

#include <dfindex/dfindex.h>

#include "Array.h"
#include "Converters.h"


BEGIN_NAMESPACE_MW_MATLAB


void getEvents(MEXInputs &inputs, MEXOutputs &outputs)
{
    boost::filesystem::path filename;
    std::set<unsigned int> event_codes;
    MWTime lower_bound, upper_bound;
    
    inputs >> filename >> event_codes >> lower_bound >> upper_bound;
    
    std::vector<int> codes;
    std::vector<MWTime> times;
    boost::container::vector<ArrayPtr> values;
    
    try {
        dfindex dfi(filename);
        DataFileIndexer::EventsIterator ei = dfi.getEventsIterator(event_codes, lower_bound,  upper_bound);
        while (true) {
            EventWrapper event = ei.getNextEvent();
            if (event.empty())
                break;
            codes.push_back(event.getEventCode());
            times.push_back(event.getTime());
            values.push_back(convertDatumToArray(Datum(event.getPayload())));
        }
    } catch (const DataFileIndexerError &e) {
        throwMATLABError("MWorks:DataFileIndexerError", e.what());
    }
    
    if (outputs.count() == 3) {
        outputs << codes;
        outputs << times;
        outputs << values;
    } else {
        const char *fieldNames[] = {"event_code", "time_us", "data"};
        ArrayPtr result(throw_if_null, mxCreateStructMatrix(1, int(codes.size()), 3, fieldNames));
        
        for (std::size_t i = 0; i < codes.size(); i++) {
            mxSetFieldByNumber(result.get(), i, 0, Array::createScalar(codes[i]).release());
            mxSetFieldByNumber(result.get(), i, 1, Array::createScalar(times[i]).release());
            mxSetFieldByNumber(result.get(), i, 2, values[i].release());
        }
        
        outputs << std::move(result);
    }
    
}


END_NAMESPACE_MW_MATLAB




























//
//  getEvents.cpp
//  MATLABTools
//
//  Created by Christopher Stawarz on 12/17/12.
//  Copyright (c) 2012 MWorks Project. All rights reserved.
//

#include "getEvents.h"

#include <MWorksCore/dfindex.h>
#include <MWorksCore/ScarabServices.h>

#include "Array.h"
#include "Converters.h"


BEGIN_NAMESPACE_MW_MATLAB


void getEvents(MEXInputs &inputs, MEXOutputs &outputs)
{
    boost::filesystem::path filename;
    std::unordered_set<int> event_codes;
    MWTime lower_bound, upper_bound;
    
    inputs >> filename >> event_codes >> lower_bound >> upper_bound;
    
    std::vector<int> codes;
    std::vector<MWTime> times;
    boost::container::vector<ArrayPtr> values;
    
    try {
        scarab::dfindex dfi(filename);
        dfi.selectEvents(event_codes, lower_bound, upper_bound);
        
        int code;
        MWTime time;
        Datum value;
        while (dfi.nextEvent(code, time, value)) {
            codes.push_back(code);
            times.push_back(time);
            values.push_back(convertDatumToArray(value));
        }
    } catch (const std::exception &e) {
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




























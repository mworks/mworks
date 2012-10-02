#include <mex.h>
#include <math.h>
#include <dfindex/dfindex.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <boost/filesystem/path.hpp>
#include <boost/static_assert.hpp>
#include "MWorksMATLABTools.h"
#include "MEXUtils.h"


const int MIN_INPUT_ARGS = 1;
const int MAX_INPUT_ARGS = 4;
const int NUM_OUTPUT_ARGS_ONE_ARRAY = 1;
const int NUM_OUTPUT_ARGS_THREE_ARRAYS = 3;


static void returnOneArray(mxArray *plhs[], DataFileIndexer::EventsIterator &ei);
static void returnThreeArrays(mxArray *plhs[], DataFileIndexer::EventsIterator &ei);


void mexFunction(int nlhs, mxArray *plhs[],
                 int nrhs, const mxArray *prhs[])
{
    if (nrhs < MIN_INPUT_ARGS || nrhs > MAX_INPUT_ARGS)
        mexErrMsgIdAndTxt(MWORKS_WRONG_NUM_INPUTS_MSG_ID,
                          "Need %d-%d input arguments:\n"
                          "%s(<data file name>, [event codes], <lower time bound>, <upper time bound>)",
                          MIN_INPUT_ARGS,
                          MAX_INPUT_ARGS,
                          mexFunctionName()
                          );
    
    if (nlhs != NUM_OUTPUT_ARGS_ONE_ARRAY && nlhs != NUM_OUTPUT_ARGS_THREE_ARRAYS)
        mexErrMsgIdAndTxt(MWORKS_WRONG_NUM_OUTPUTS_MSG_ID,
                          "Need %d or %d output arguments:\n"
                          "events = %s(...)\n"
                          "[codes, times, values] = %s(...)",
                          NUM_OUTPUT_ARGS_ONE_ARRAY,
                          NUM_OUTPUT_ARGS_THREE_ARRAYS,
                          mexFunctionName(),
                          mexFunctionName());
    
    std::string filename;
    getStringParameter(prhs, 1, filename);
    boost::filesystem::path mwk_file(filename);
    
    std::vector<unsigned int> event_codes;
    if (nrhs >= 2) {
        size_t numCodes;
        const double *codes = getNumericArrayParameter(prhs, 2, numCodes);
        for (size_t i = 0; i < numCodes; i++) {
            event_codes.push_back((unsigned int)(codes[i]));
        }
    }
    
    MWTime lower_bound = MIN_MONKEY_WORKS_TIME();
    if(nrhs >= 3) {
        lower_bound = MWTime(getNumericScalarParameter(prhs, 3));
    }
    
    MWTime upper_bound = MAX_MONKEY_WORKS_TIME();
    if(nrhs >= 4) {
        upper_bound = MWTime(getNumericScalarParameter(prhs, 4));
    }
    
    dfindex dfi(mwk_file);
    DataFileIndexer::EventsIterator ei = dfi.getEventsIterator(event_codes, lower_bound,  upper_bound);
    
    if (nlhs == NUM_OUTPUT_ARGS_ONE_ARRAY) {
        returnOneArray(plhs, ei);
    } else {
        returnThreeArrays(plhs, ei);
    }
}


static void returnOneArray(mxArray *plhs[], DataFileIndexer::EventsIterator &ei) {
    std::vector<MATLABEventInfo> events;
    EventWrapper event;
    
    while ((event = ei.getNextEvent())) {
        events.push_back(MATLABEventInfo(event.getDatum()));
    }
    
    mxArray *eventsStruct = createTopLevelEventStruct(events.size());
    
    for (int i = 0; i < events.size(); i++) {
        insertEventIntoEventList(eventsStruct, i, events[i]);
    }
    
    plhs[0] = eventsStruct;
}


static void returnThreeArrays(mxArray *plhs[], DataFileIndexer::EventsIterator &ei) {
    std::vector<int> codes;
    std::vector<MWTime> times;
    std::vector<mxArray *> values;
    
    EventWrapper event;
    
    while ((event = ei.getNextEvent())) {
        codes.push_back(event.getEventCode());
        times.push_back(event.getTime());
        values.push_back(getScarabDatum(event.getPayload()));
    }
    
    mxArray *codesArray = mxCreateNumericMatrix(1, codes.size(), mxINT32_CLASS, mxREAL);
    memcpy(mxGetData(codesArray), &(codes.front()), codes.size() * sizeof(int));
    
    BOOST_STATIC_ASSERT(sizeof(MWTime) == sizeof(long long));
    mxArray *timesArray = mxCreateNumericMatrix(1, times.size(), mxINT64_CLASS, mxREAL);
    memcpy(mxGetData(timesArray), &(times.front()), times.size() * sizeof(MWTime));
    
    mxArray *valuesArray = mxCreateCellMatrix(1, values.size());
    for (size_t i = 0; i < values.size(); i++) {
        mxSetCell(valuesArray, i, values[i]);
    }
    
    plhs[0] = codesArray;
    plhs[1] = timesArray;
    plhs[2] = valuesArray;
}



























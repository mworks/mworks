#include "mex.h"
#include <math.h>
#include "dfindex/dfindex.h"
#include <sys/time.h>
#include <unistd.h>
#include "boost/filesystem/path.hpp"
#include "mWorksStreamUtilities.h"
#include <iostream>

using namespace std;

const int MIN_INPUT_ARGS = 1;
const int MAX_INPUT_ARGS = 4;
const int NUM_OUTPUT_ARGS = 1;

// Function definitions.
// -----------------------------------------------------------------
void mexFunction (int nlhs, mxArray *plhs[],
		  int nrhs, const mxArray *prhs[]) {
  
  // Check to see if we have the correct number of input and output
  // arguments.
  if (nrhs > MAX_INPUT_ARGS || nrhs < MIN_INPUT_ARGS)
    mexErrMsgTxt("needs 1-4 arguments: events(<data file name>, [event codes], <lower time bound>, <upper time bound>)");
  if (nlhs != NUM_OUTPUT_ARGS)
    mexErrMsgTxt("only had one output argument");

  // Get the inputs.
  boost::filesystem::path mwk_file(getString(prhs[0]));
  std::vector<unsigned int> event_codes;

  MWTime lower_bound = MIN_MONKEY_WORKS_TIME();
  MWTime upper_bound = MAX_MONKEY_WORKS_TIME();

  if (nrhs >= 2) {
      if (!mxIsNumeric(prhs[1])) {
          mexErrMsgTxt("argument 2 must be a numeric array");
      }

      size_t numElements = mxGetNumberOfElements(prhs[1]);
      double *arrayData = mxGetPr(prhs[1]);
      
      for (size_t i = 0; i < numElements; i++) {
          event_codes.push_back((unsigned int)(*(arrayData + i)));
      }
  }

  if(nrhs >= 3) {
    lower_bound = getMWorksTime(prhs[2]);
  }

  if(nrhs >= 4) {
    upper_bound = getMWorksTime(prhs[3]);
  }

  dfindex dfi(mwk_file);
  DataFileIndexer::EventsIterator ei = dfi.getEventsIterator(event_codes, lower_bound,  upper_bound);
  std::vector<MATLABEventInfo> events;
  EventWrapper event;

  while ((event = ei.getNextEvent())) {
      events.push_back(MATLABEventInfo(event.getDatum()));
  }
  
  mxArray *events_struct = createTopLevelEventStruct(events.size());
  
  for (int i = 0; i < events.size(); i++) {
      insertEventIntoEventList(events_struct, i, events[i]);
  }
  
  // for now output the number of events
  plhs[0] = events_struct;
}

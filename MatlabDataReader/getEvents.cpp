#include "mex.h"
#include <math.h>
#include "dfindex/dfindex.h"
#include <sys/time.h>
#include <unistd.h>
#include "boost/filesystem/path.hpp"
#include "MonkeyWorksStreamUtilities/monkeyWorksStreamUtilities.h"
#include <iostream>

using namespace std;

extern void _main();

const int MIN_INPUT_ARGS = 2;
const int MAX_INPUT_ARGS = 4;
const int NUM_OUTPUT_ARGS = 1;

// Function declarations.
// -----------------------------------------------------------------
std::vector<unsigned int> arrayToVector(const mxArray *array);
mxArray *createTopLevelEventStruct(long nevents);

// Function definitions.
// -----------------------------------------------------------------
void mexFunction (int nlhs, mxArray *plhs[],
		  int nrhs, const mxArray *prhs[]) {
  
  // Check to see if we have the correct number of input and output
  // arguments.
  if (nrhs > MAX_INPUT_ARGS || nrhs < MIN_INPUT_ARGS)
    mexErrMsgTxt("needs 2-4 arguments events(<data file name>, [event codes], <lower time bound>, <upper time bound>");
  if (nlhs != NUM_OUTPUT_ARGS)
    mexErrMsgTxt("only had one output argument");

  // Get the inputs.
  boost::filesystem::path mwk_file(getString(prhs[0]), boost::filesystem::native);
  std::vector<unsigned int> event_codes(arrayToVector(prhs[1]));

  MonkeyWorksTime lower_bound = MIN_MONKEY_WORKS_TIME();
  MonkeyWorksTime upper_bound = MAX_MONKEY_WORKS_TIME();

  if(nrhs >= 3) {
    lower_bound = getMonkeyWorksTime(prhs[2]);
  }

  if(nrhs >= 4) {
    upper_bound = getMonkeyWorksTime(prhs[3]);
  }

  vector<ScarabDatumWrapper> events;
  
  dfindex dfi(mwk_file);
  events = dfi.events(event_codes, lower_bound,  upper_bound);
  
  mxArray *events_struct = createTopLevelEventStruct(events.size());
  
  int event_number = 0;
  for(std::vector<ScarabDatumWrapper>::const_iterator i = events.begin();
      i != events.end();
      ++i) {
    ScarabDatum *datum = i->getDatum();
    //    mxArray *event = getScarabEventData(datum);
    insertDatumIntoEventList(events_struct, event_number, datum);
    
    event_number++;
  }
  
  // for now output the number of events
  plhs[0] = events_struct;
}

std::vector<unsigned int> arrayToVector(const mxArray *array) {
  std::vector<unsigned int>return_vector;

  if(!mxIsNumeric(array)) {
    mexErrMsgTxt("Could not create vector");
    return return_vector;
  }

  for(int i = 0; i < mxGetNumberOfElements(array); ++i) {
    double *event_code = mxGetPr(array)+i;
    //    mexPrintf("i=%d .. num elem=%d .. cur elem: %ud\n", i, mxGetNumberOfElements(array), (unsigned int)*event_code);
    return_vector.push_back((unsigned int)*event_code);
  }
    
  return return_vector;
}

mxArray *createTopLevelEventStruct(const long nevents) {
  // *****************************************************************
  // Allocate storage for the number of events we are about to read
  // *****************************************************************
  
  const char *event_field_names[] = {"event_code", "time_us", "data"};
  int event_nfields = 3;
  mwSize event_dims = nevents;
  mxArray *events = mxCreateStructArray(1, &event_dims, 
					event_nfields, event_field_names);
  
  return events;
}



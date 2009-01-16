#include "mex.h"
#include <math.h>
#include "dfindex/dfindex.h"
#include <sys/time.h>
#include <unistd.h>
#include "boost/filesystem/path.hpp"
#include "MonkeyWorksStreamUtilities/monkeyWorksStreamUtilities.h"

using namespace std;

extern void _main();

const int NUM_INPUT_ARGS = 1;
const int NUM_OUTPUT_ARGS = 1;

// Function declarations.
// -----------------------------------------------------------------
mxArray *createTopLevelCodecStruct(long nevents);

// Function definitions.
// -----------------------------------------------------------------
void mexFunction (int nlhs, mxArray *plhs[],
		  int nrhs, const mxArray *prhs[]) {
  
  // Check to see if we have the correct number of input and output
  // arguments.
  if (nrhs != NUM_INPUT_ARGS)
    mexErrMsgTxt("only one input arguements: codecs(<data file name>");
  if (nlhs != NUM_OUTPUT_ARGS)
    mexErrMsgTxt("only had one output argument");

  // Get the inputs.
  boost::filesystem::path mwk_file(getString(prhs[0]), boost::filesystem::native);
  std::vector<unsigned int> event_codes;
  event_codes.push_back(0);

  MonkeyWorksTime lower_bound = MIN_MONKEY_WORKS_TIME();
  MonkeyWorksTime upper_bound = MAX_MONKEY_WORKS_TIME();
  
  dfindex dfi(mwk_file);
  
  vector<ScarabDatumWrapper> codecs = dfi.events(event_codes, lower_bound,  upper_bound);
  
  mxArray *codec_struct = createTopLevelCodecStruct(codecs.size());
  
  int codec_number = 0;
  for(std::vector<ScarabDatumWrapper>::const_iterator i = codecs.begin();
      i != codecs.end();
      ++i) {
    ScarabDatum *datum = i->getDatum();
    insertDatumIntoCodecList(codec_struct, codec_number, datum);
    
    codec_number++;
  }
  
  // for now output the codecs
  plhs[0] = codec_struct;
}

mxArray *createTopLevelCodecStruct(const long ncodecs) {
  // *****************************************************************
  // Allocate storage for the number of codecs we are about to read
  // *****************************************************************
  
  const char *codec_field_names[] = {"time_us", "codec"};
  int codec_nfields = 2;
  mwSize codec_dims = ncodecs;
  mxArray *codecs = mxCreateStructArray(1, &codec_dims, 
					codec_nfields, codec_field_names);
  
  return codecs;
}



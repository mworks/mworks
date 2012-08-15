#include "mex.h"
#include <math.h>
#include "dfindex/dfindex.h"
#include <sys/time.h>
#include <unistd.h>
#include "boost/filesystem/path.hpp"
#include "MWorksMATLABTools.h"
#include "MEXUtils.h"

using namespace std;

const int NUM_INPUT_ARGS = 1;
const int NUM_OUTPUT_ARGS = 1;

void mexFunction (int nlhs, mxArray *plhs[],
		  int nrhs, const mxArray *prhs[]) {
  
  // Check to see if we have the correct number of input and output
  // arguments.
  if (nrhs != NUM_INPUT_ARGS)
    mexErrMsgTxt("only one input argument: codecs(<data file name>)");
  if (nlhs != NUM_OUTPUT_ARGS)
    mexErrMsgTxt("only had one output argument");

  // Get the inputs.
  std::string filename;
  getStringParameter(prhs, 1, filename);
  boost::filesystem::path mwk_file(filename);

  std::vector<unsigned int> event_codes;
  event_codes.push_back(0);

  MWTime lower_bound = MIN_MONKEY_WORKS_TIME();
  MWTime upper_bound = MAX_MONKEY_WORKS_TIME();
  
  dfindex dfi(mwk_file);
  
  vector<EventWrapper> codecs;
  dfi.getEvents(codecs, event_codes, lower_bound,  upper_bound);
  
  mxArray *codec_struct = createTopLevelCodecStruct(codecs.size());
  
  int codec_number = 0;
  for(std::vector<EventWrapper>::const_iterator i = codecs.begin();
      i != codecs.end();
      ++i) {
    ScarabDatum *datum = i->getDatum();
    insertDatumIntoCodecList(codec_struct, codec_number, datum);
    
    codec_number++;
  }
  
  // for now output the codecs
  plhs[0] = codec_struct;
}

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include <vector>
#include "Scarab/scarab.h"
#include "Scarab/scarab_utilities.h"
#include "MonkeyWorksCore/EventConstants.h"
#include "mat.h"
#include "matrix.h"
#include "monkeyWorksStreamUtilities.h"


#define SCARAB_SYSTEM_EVENT_PACKAGE_TYPE_INDEX 0
#define SCARAB_SYSTEM_EVENT_CONTROL_TYPE_INDEX 1
#define SCARAB_SYSTEM_EVENT_PAYLOAD_INDEX 2
#define SCARAB_SYSTEM_EVENT_N_PAYLOAD_ELEMENTS 3

#define ml_FILENAME "filename"
#define ml_EVENT_CODEC "event_codec"
#define ml_EVENTS "events"

ScarabSession *dfrConnectToFile(const std::string &file);
std::map<std::string, int> dfrGetVariables(const std::string &file);
mxArray *createTopLevelDataStruct(const std::string &name);
mxArray *createTopLevelEventStruct(const long nevents);

int main( int argc, char *argv[])
{
  using namespace std;
  
  if ( argc < 4 ) {
    std::cout << "usage: " << argv[0] << " <in filename> <out filename> <number of events per block> [event to include 1] [event to include 2] ... [event to include n]" << std::endl;
  } else {
    ifstream the_file ( argv[1] );
    if ( !the_file.is_open() ) {
      std::cout << "Could not open " << argv[1] << std::endl;
    } else {
      string filename(argv[1]);
      int count = 0;
      string outfile(argv[2]);

      std::map<std::string, int> variables = dfrGetVariables(filename);
      
      std::cout << "*** Including:" << std::endl;
      std::vector<int> eventsToInclude;
      eventsToInclude.push_back(RESERVED_CODEC_CODE); // always include the codec                                                                                                                                                                                                                           
      if(argc > 4) {
        for(int i=4; i < argc; ++i) {
          eventsToInclude.push_back(variables[std::string(argv[i])]);
	  std::cout << "***           " << argv[i] << std::endl;
        }
      } else {
        for(std::map<std::string, int>::const_iterator i = variables.begin();
            i != variables.end();
            ++i) {
          eventsToInclude.push_back(i->second);
	  std::cout << "***           " << i->first << std::endl;
        }
      }

      std::sort(eventsToInclude.begin(), eventsToInclude.end());

      // first count the number of events
      {
	ScarabSession *session = dfrConnectToFile(filename);
	ScarabDatum *datum;
	while(datum = scarab_read(session)){
	  // All events should be scarab lists
	  if(datum->type != SCARAB_LIST){  
	    std::cout << "Invalid event structure..." << std::endl;
	    break;
	  }

	  const int code = getScarabEventCode(datum);
	  if(std::binary_search(eventsToInclude.begin(), eventsToInclude.end(), code)) {
	    count++;
	  }

	  if(count % 1000000 == 0){
	    std::cout << "... " << count << " events" << std::endl;
	  }
	  
	  scarab_free_datum(datum);
	  
	}
	
	scarab_session_close(session);
	cout << "Processing " << count << " events" << std::endl; 
      }
      
      int events_per_block = 0;
      if(atoi(argv[3]) > 0) {
	events_per_block = atoi(argv[3]);
      } else {
	events_per_block = count;
      }
      
      {
	ScarabSession *session = dfrConnectToFile(filename);
	
	int curr_block = 0;
	int read_count = 0;
	int nread = 0;
	do {	  
	  int nevents = (count < events_per_block) ? count : events_per_block; 
	  
	  // second create a event structure that will hold the number of events
	  mxArray *data_struct = createTopLevelDataStruct(filename);
	  
	  // create the event struct
	  mxArray *events_struct = createTopLevelEventStruct(nevents);
	  
	  mxSetField(data_struct, 0, ml_EVENTS, events_struct);
	  
	  
	  nread = 0;
	  
	  while(nevents > 0) {
	    ScarabDatum *datum = scarab_read(session);
	    
	    
	    // All events should be scarab lists
	    if(datum->type != SCARAB_LIST){  
	      std::cout << "Invalid event structure.  Quitting." << std::endl;
	      scarab_free_datum(datum);
	      break;
	    }
	    
	    const int code = getScarabEventCode(datum);
	    if(std::binary_search(eventsToInclude.begin(), eventsToInclude.end(), code)) {
	      read_count++;
	      
	      
	      // Convert
	      scarabEventToDataStruct(data_struct, nread, datum);
	      
	      
	      nread++;
	      
	      if(nread % 500000 == 0){
		cout << read_count << " (read_count) events processed so far..." << std::endl;
	      }
	      nevents--;	  
	    }  
	    scarab_free_datum(datum);
	  }
	  
	  if(nread > 0) {
	    stringstream lStream; 
	    lStream << curr_block;
	    string cur_outfile(outfile + lStream.str() + ".mat");
	    
	    std::cout << "Writing " << nread << " events to file: " << cur_outfile << std::endl;
	    
	    MATFile *output = matOpen(cur_outfile.c_str(), "w");
	    matPutVariable(output, "events", data_struct);
	    matClose(output);
	  }
	  
	  curr_block++;
	  count -= nread;
	  mxDestroyArray(data_struct);
	  
	} while(nread > 0 && count > 0);

	scarab_session_close(session);
      }
    }
  }  
  return 0;
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

mxArray *createTopLevelDataStruct(const std::string &name) {
  // *****************************************************************
  // Create the file struct
  // events field will contain the actual events
  // event_types field will include a code that identifies the
  //   event types (e.g. message, data, etc.)
  // event_codec field will contain the event name / code dictionary
  // *****************************************************************
  
  mwSize ndims = 1;
  mwSize data_dims = 1;
  const char *data_field_names[] = {ml_FILENAME, ml_EVENT_CODEC, ml_EVENTS};
  int data_nfields = 3; // filename, codec, event
  mxArray *dataStruct = mxCreateStructArray(ndims, &data_dims, data_nfields, 
					    data_field_names);
  
  mxArray *old_filename = mxGetField(dataStruct, 0, ml_FILENAME);
  if(old_filename){
    mxDestroyArray(old_filename);
  }
  mxSetField(dataStruct, 
	     0, 
	     ml_FILENAME, 
	     mxCreateString(name.c_str()));
  
  return dataStruct;
}



ScarabSession *dfrConnectToFile(const std::string &file) {
  using namespace std;
  
  string uri = "ldobinary:file://" + file;
  
  std::cerr << "opening uri: " << uri << std::endl;
  scarab_init();
  
  
  // I hate myself for this
  char *stupid_crap = new char[uri.length() + 1];
  strncpy(stupid_crap, uri.c_str(), uri.length() + 1);
  ScarabSession *session = scarab_session_connect(stupid_crap);
  
  delete [] stupid_crap;
  return session;
}


std::map<std::string, int> dfrGetVariables(const std::string &file) {
  std::map<std::string, int> retval;

  ScarabSession *session = dfrConnectToFile(file);

  ScarabDatum *datum = 0;
  while(datum = scarab_read(session)) {
    if(datum->type != SCARAB_LIST){
      std::cerr << "Invalid event structure.  Quitting." << std::endl;
      scarab_free_datum(datum);
      exit(-1);
    }

    if(getScarabEventCode(datum) == RESERVED_CODEC_CODE) {

      ScarabDatum *codec = getScarabEventPayload(datum);
      int n_codec_entries = codec->data.dict->tablesize;
      ScarabDatum **keys = scarab_dict_keys(codec);

      for(int c = 0; c < n_codec_entries; ++c){
	if(keys[c]) {
	  int code = keys[c]->data.integer;
	  ScarabDatum *serializedVar = scarab_dict_get(codec, keys[c]);

	  if(serializedVar && serializedVar->type == SCARAB_DICT) {
	    ScarabDatum *name_key = scarab_new_string("tagname");
	    ScarabDatum *tagnameDatum = scarab_dict_get(serializedVar, name_key);
	    scarab_free_datum(name_key);

	    if(tagnameDatum) {
	      std::string tagname(scarab_extract_string(tagnameDatum));
	      retval[tagname] = code;
	    } else {
	      std::cerr << "Variable doesn't have a tagname.  Exiting." << std::endl;
	    }
	  }
	}
      }
      scarab_free_datum(datum);
      break;
    }

    scarab_free_datum(datum);
  }

  scarab_session_close(session);

  return retval;
}

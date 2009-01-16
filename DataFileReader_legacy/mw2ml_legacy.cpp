#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include "Scarab/scarab.h"
#include "MonkeyWorksCore/Event.h"
#include "MonkeyWorksCore/MessageEvent.h"
#include "MonkeyWorksCore/SystemEvent.h"
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

ScarabSession *dfrConnectToFile(const std::string file);
mxArray *createTopLevelDataStruct(const std::string name);
mxArray *createTopLevelEventStruct(long nevents);

int main( int argc, char *argv[])
{
  using namespace std;
  
  if ( argc < 3 && argc > 4) {
    cout << "usage: " << argv[0] << " <in filename> <out filename> [number of events per block]" << endl;
  } else {
    ifstream the_file ( argv[1] );
    if ( !the_file.is_open() ) {
      cout << "Could not open " << argv[1] << endl;
    } else {
      string filename(argv[1]);
      ScarabSession *session = dfrConnectToFile(filename);
      ScarabDatum *datum;
      int count = 0;
      string outfile(argv[2]);

      // first count the number of events
      while(datum = scarab_read(session)){
	// All events should be scarab lists
	if(datum->type != SCARAB_LIST){  
	  cout << "Invalid event structure..." << endl;
	  break;
	}
	count++;
	if(count % 250000 == 0){
	  cout << "... " << count << " events" << endl;
	}
	
	scarab_free_datum(datum);
      }
      scarab_session_close(session);

      cout << "Processing " << count << " events" << endl; 
      

      int events_per_block = 0;
      if(argc == 4) {
        events_per_block = atoi(argv[3]);
      } else {
        events_per_block = count;
      }

      session = dfrConnectToFile(filename);


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
	
	while((datum = scarab_read(session)) && nevents > 0) {
	  
	  read_count++;
	  // All events should be scarab lists
	  if(datum->type != SCARAB_LIST){  
	    cout << "Invalid event structure.  Quitting." << endl;
	    scarab_free_datum(datum);
	    break;
	  }

	

	  // Convert
	  scarabEventToDataStruct(data_struct, nread, datum);
	  
	  
	  nread++;
	  
	  if(read_count % 500000 == 0){
	    cout << "... " << read_count << endl;
	  }
	  
	  scarab_free_datum(datum);
	  nevents--;
	}

	if(nread > 0) {
	  stringstream lStream; 
	  lStream << curr_block;
	  string cur_outfile(outfile + lStream.str() + ".mat");
	  
	  cout << "Writing: " << cur_outfile << endl;
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

mxArray *createTopLevelEventStruct(long nevents) {
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

mxArray *createTopLevelDataStruct(const std::string name) {
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



ScarabSession *dfrConnectToFile(const std::string file) {
  using namespace std;
  
  string uri = "ldobinary:file://" + file;
  
  cerr << "opening uri: " << uri << endl;
  scarab_init();
  
  
  // I hate myself for this
  char *stupid_crap = new char[uri.length() + 1];
  strncpy(stupid_crap, uri.c_str(), uri.length() + 1);
  return scarab_session_connect(stupid_crap);
}

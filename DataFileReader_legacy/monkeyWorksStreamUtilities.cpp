/*
 *  monkeyWorksStreamUtilities.cpp
 *  MonkeyWorksMatlab
 *
 *  Created by David Cox on 12/20/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#include <string>
#include "monkeyWorksStreamUtilities.h"


double scarab_extract_float(ScarabDatum *datum){
  
  
#if	__LITTLE_ENDIAN__
  return *((double *)(datum->data.opaque.data));
#else
  int i;
  unsigned char swap_buffer[sizeof(double)];
  unsigned char *datum_bytes = (unsigned char *)datum->data.opaque.data;
  for(i = 0; i < sizeof(double); i++){
    swap_buffer[i] = datum_bytes[sizeof(double) - i - 1];
  }
  
  return *((double *)swap_buffer);
#endif
}

int getScarabEventCode(ScarabDatum *datum){
  
  ScarabDatum *code_datum = scarab_list_get(datum, SCARAB_EVENT_CODEC_CODE_INDEX);
  return code_datum->data.integer;
  
}

long long getScarabEventTime(ScarabDatum *datum){
  ScarabDatum *time_datum = scarab_list_get(datum, SCARAB_EVENT_TIME_INDEX);
  return time_datum->data.integer;
}

ScarabDatum *getScarabEventPayload(ScarabDatum *datum){
  ScarabDatum *payload_datum = scarab_list_get(datum, SCARAB_EVENT_PAYLOAD_INDEX);
  return payload_datum;
}



mxArray *recursiveGetScarabDict(ScarabDatum *datum);
mxArray *recursiveGetScarabList(ScarabDatum *datum){
  
  int i;
  
  //mexPrintf("recursiveGetScarabList");
  
  if(datum->type != SCARAB_LIST){
    return NULL;
  }
  
  int n = datum->data.list->size;
  ScarabDatum **values = datum->data.list->values;
  
  
  mxArray *cell_matrix = mxCreateCellMatrix(1, n);
  
  for(i = 0; i < n; i++){
    
    mxArray *mx_datum;
    
    if(values[i] != NULL){
      
      switch(values[i]->type){
	
      case SCARAB_INTEGER:
	mx_datum = mxCreateDoubleScalar((double)values[i]->data.integer);
	break;
      case SCARAB_FLOAT:
	mx_datum = mxCreateDoubleScalar((double)values[i]->data.floatp);
	break;
      case SCARAB_FLOAT_OPAQUE:
	mx_datum = mxCreateDoubleScalar(scarab_extract_float(values[i]));
	break;
      case SCARAB_OPAQUE:
	{
	  std::string value(scarab_extract_string(values[i]));
	  mx_datum = mxCreateString(value.c_str());
	  break;
	}
      case SCARAB_DICT:
	mx_datum = recursiveGetScarabDict(values[i]);
	break;
      case SCARAB_LIST:
	mx_datum = recursiveGetScarabList(values[i]);
	break;
      default:
	mx_datum = NULL;
	break;
      }
      
      if(mx_datum != NULL){
	mxSetCell(cell_matrix, i, mx_datum);
      }
    }
	}
  
  return cell_matrix;
}


mxArray *recursiveGetScarabDict(ScarabDatum *datum){
  
  int i;
  
  //mexPrintf("recursiveGetScarabDict");
  
  if(datum->type != SCARAB_DICT){
    return NULL;
  }
  
  int n = scarab_dict_number_of_elements(datum);
  ScarabDatum **keys = scarab_dict_keys(datum);
  ScarabDatum **values = scarab_dict_values(datum);
  
  char **fields = (char **) calloc(n, sizeof(char *));
  
  for(i = 0; i < n; i++){
    if(keys[i]->type == SCARAB_OPAQUE){
      fields[i] = (char *)scarab_extract_string(keys[i]);
    }
  }
  
  mwSize dims = 1;
  mwSize ndims = 1;
  mxArray *struct_array = mxCreateStructArray(ndims, &dims, n, (const char **)fields);
  
  for(i = 0; i < n; i++){
    
    mxArray *mx_datum;
    
    if(values[i] != NULL){
      
      switch(values[i]->type){
	
      case SCARAB_INTEGER:
	mx_datum = mxCreateDoubleScalar((double)values[i]->data.integer);
	break;
      case SCARAB_FLOAT:
	mx_datum = mxCreateDoubleScalar((double)values[i]->data.floatp);
	break;
      case SCARAB_FLOAT_OPAQUE:
	mx_datum = mxCreateDoubleScalar(scarab_extract_float(values[i]));
	break;
      case SCARAB_OPAQUE:
	{
	  std::string value(scarab_extract_string(values[i]));
	  mx_datum = mxCreateString(value.c_str());
	  break;
	}
      case SCARAB_DICT:
	mx_datum = recursiveGetScarabDict(values[i]);
	break;
      case SCARAB_LIST:
	mx_datum = recursiveGetScarabList(values[i]);
	break;
      default:
	mx_datum = NULL;
	break;
      }
      
      if(mx_datum != NULL){
	mxArray *old_thing = mxGetField(struct_array, 0, fields[i]);
	if(old_thing){
	  mxDestroyArray(old_thing);
	}
	
	mxSetField(struct_array, 0, fields[i], mx_datum);
      }
    }
  }
  
  return struct_array;
}

mxArray *getScarabEventData(ScarabDatum *datum){
  if(datum == NULL){
    return mxCreateDoubleScalar(0.0);
  }
  
  ScarabDatum *payload = getScarabEventPayload(datum);
  
  if(!payload){
    return mxCreateDoubleScalar(0.0);
  }
  
  mxArray *retVal;

  switch(payload->type) {
  case SCARAB_INTEGER:
    retVal =  mxCreateDoubleScalar(payload->data.integer);
    break;
  case SCARAB_FLOAT:
    retVal = mxCreateDoubleScalar((double)payload->data.floatp);
    break;
  case SCARAB_FLOAT_NAN:
  case SCARAB_FLOAT_OPAQUE:
    retVal = mxCreateDoubleScalar(scarab_extract_float(payload)); // HACK
    break;
  case SCARAB_OPAQUE:
    {
      std::string strPayload(scarab_extract_string(payload));
      retVal = mxCreateString(strPayload.c_str());
      break;
    }
  case SCARAB_DICT:
    retVal = recursiveGetScarabDict(payload);
    break;
  case SCARAB_LIST:
    retVal = recursiveGetScarabList(payload);
    break;
  default:
    retVal = mxCreateDoubleScalar(0.0);
    break;
  }


  return retVal;
}


mxArray *extractCodec(ScarabDatum *datum){
  ScarabDatum *codec = getScarabEventPayload(datum);
  
  int n_codec_entries = codec->data.dict->tablesize;
  
  ScarabDatum **keys = scarab_dict_keys(codec);
  const char *codec_field_names[] = {"code", 
				     "tagname",
				     "logging",  
				     "defaultvalue", 
				     "shortname", 
				     "longname",
				     "editable", 
				     "nvals", 
				     "domain", 
				     "viewable", 
				     "persistant"}; // more to come later?
  int n_codec_fields = 11;
  mwSize codec_size = n_codec_entries;
  mxArray *codec_struct = mxCreateStructArray(1, 
					      &codec_size,
					      n_codec_fields, 
					      codec_field_names);
  
  

  for(int c = 0; c < n_codec_entries; ++c){
    if(keys[c]) {
      int code = keys[c]->data.integer;
      ScarabDatum *serializedVar = scarab_dict_get(codec, keys[c]);

      mxSetField(codec_struct, c, "code", mxCreateDoubleScalar(code));
      
      if(serializedVar && serializedVar->type == SCARAB_DICT) {      
	for(int d = 0; d < serializedVar->data.dict->tablesize; ++d) {
	  
	  ScarabDatum **varKeys = scarab_dict_keys(serializedVar);
	  
	  if(varKeys[d]) {
	    std::string thiskey(scarab_extract_string(varKeys[d]));
	    
	    ScarabDatum *varValue = scarab_dict_get(serializedVar, varKeys[d]);
	    if(varValue == NULL){
	      
	    } else if(varValue->type == SCARAB_INTEGER){
	      mxSetField(codec_struct, c, thiskey.c_str(), 
			 mxCreateDoubleScalar(varValue->data.integer));
	    } else if(varValue->type == SCARAB_OPAQUE){
	      std::string val(scarab_extract_string(varValue));
	      mxSetField(codec_struct, c, thiskey.c_str(), 
			 mxCreateString(val.c_str()));
	    }
	  }
	}      
      }
    }    
   }
   return codec_struct;

}

int scarabEventToDataStruct(mxArray *data_struct, int index, ScarabDatum *datum){
  
  mxArray *eventlist = mxGetField(data_struct, 0, "events");
  
  long code = getScarabEventCode(datum);
  long long time = getScarabEventTime(datum);
  
  mxArray *data;
  
  if(code == 0) { // codec arrived
    mxArray *codec = extractCodec(datum);
    
    mxArray *old_codec = mxGetField(data_struct, 0 , "event_codec");
    if(old_codec != 0) {
      mxDestroyArray(old_codec);
    }
    
    mxSetField(data_struct, 0, "event_codec", codec);
    
    data = mxDuplicateArray(codec);
  } else {
    data = getScarabEventData(datum);
  } 
  
  mxArray *old_code = mxGetField(eventlist, index, "event_code");
  if(old_code != NULL){
    mxDestroyArray(old_code);
  }
  mxSetField(eventlist, index, "event_code", mxCreateDoubleScalar((double)code));		
  
  mxArray *old_time = mxGetField(eventlist, index, "time_us");
  if(old_time != NULL){
    mxDestroyArray(old_time);
  }
  
  mxSetField(eventlist, index, (const char *)"time_us", mxCreateDoubleScalar((double)time));
  
  mxArray *old_data = mxGetField(eventlist, index, "data");
  if(old_data){
    mxDestroyArray(old_data);
  }
  mxSetField(eventlist, index, (const char *)"data", data);
  
  return code;
}	

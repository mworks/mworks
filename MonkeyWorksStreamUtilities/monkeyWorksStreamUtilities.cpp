/*
 *  monkeyWorksStreamUtilities.cpp
 *  MonkeyWorksMatlab
 *
 *  Created by David Cox on 12/20/06.
 *  Copyright 2006 MIT. All rights reserved.
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
					char *buffer = scarab_extract_string(values[i]);
					mx_datum = mxCreateString(buffer);
					free(buffer);
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
	
	int dictionaries = 0;
	int lists = 0;
	int floats = 0;
	int unknowns = 0;	
	
	// fix me here
	for(i = 0; i < n; i++){
		if(keys[i]) {
			switch(keys[i]->type) {
				case SCARAB_OPAQUE:
					//mexPrintf("  key is SCARAB_OPAQUE (keys[%d]->type): %d\n", i, keys[i]->type);
					fields[i] = (char *)scarab_extract_string(keys[i]);
					break;
				case SCARAB_DICT:
					//mexPrintf("  key is SCARAB_DICT (keys[%d]->type): %d\n", i, keys[i]->type);
				{
					const char *dictPrefix = "dict";
					++dictionaries;
					int len = snprintf(0, 0, "%s%d", dictPrefix, dictionaries);
					char *str = (char *)malloc((len + 1) * sizeof(char));
					
					len = snprintf(str, len + 1, "%s%d", dictPrefix, dictionaries);
					
					fields[i]=str;
				}
					break;
				case SCARAB_LIST:
					//mexPrintf("  key is SCARAB_LIST (keys[%d]->type): %d\n", i, keys[i]->type);
				{
					const char *listPrefix = "list";
					++lists;
					int len = snprintf(0, 0, "%s%d", listPrefix, lists);
					char *str = (char *)malloc((len + 1) * sizeof(char));
					
					len = snprintf(str, len + 1, "%s%d", listPrefix, lists);
					
					fields[i]=str;
				}
					break;
				case SCARAB_FLOAT:
				case SCARAB_FLOAT_INF:
				case SCARAB_FLOAT_NAN:
				case SCARAB_FLOAT_OPAQUE:
					//mexPrintf("  key is SCARAB_FLOAT (keys[%d]->type): %d\n", i, keys[i]->type);
				{
					const char *floatPrefix = "float";
					++floats;
					int len = snprintf(0, 0, "%s%d", floatPrefix, floats);
					char *str = (char *)malloc((len + 1) * sizeof(char));
					
					len = snprintf(str, len + 1, "%s%d", floatPrefix, floats);
					
					fields[i]=str;
				}
					break;
				case SCARAB_INTEGER:
					//mexPrintf("  key is SCARAB_INTEGER (keys[%d]->type): %d\n", i, keys[i]->type);
				{
					const char *intPrefix = "int";
					long long intValue = keys[i]->data.integer;
					int len = snprintf(0, 0, "%s%lld", intPrefix, intValue);
					char *str = (char *)malloc((len + 1) * sizeof(char));
					
					len = snprintf(str, len + 1, "%s%lld", intPrefix, intValue);
					
					fields[i]=str;
				}
					break;
				default:
					//mexPrintf("  key is unknown (keys[%d]->type): %d\n", i, keys[i]->type);
				{
					const char *unknownPrefix = "unknown";
					++unknowns;
					int len = snprintf(0, 0, "%s%d", unknownPrefix, unknowns);
					char *str = (char *)malloc((len + 1) * sizeof(char));
					
					len = snprintf(str, len + 1, "%s%d", unknownPrefix, unknowns);
					
					fields[i]=str;
				}
					break;
			}
		} else {
			fields[i] = NULL;
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
					char *buffer = scarab_extract_string(values[i]);
					mx_datum = mxCreateString(buffer);
					free(buffer);
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
	
	
	for(i = 0; i < n; i++){
		if(fields[i]) {
			free(fields[i]);
		}
	}
	free(fields);
	
	return struct_array;
}

MonkeyWorksTime getMonkeyWorksTime(const mxArray *time) {
	if(!mxIsNumeric(time)) {
		return -1;
	}
	
	return (MonkeyWorksTime)mxGetScalar(time);
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
			char *buffer = scarab_extract_string(payload);
			retVal = mxCreateString(buffer);
			free(buffer);
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


mxArray *getCodec(ScarabDatum *datum){
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
						char *buffer = scarab_extract_string(varKeys[d]);
						
						ScarabDatum *varValue = scarab_dict_get(serializedVar, varKeys[d]);
						if(varValue == NULL){
							
						} else if(varValue->type == SCARAB_INTEGER){
							mxSetField(codec_struct, c, buffer, 
									   mxCreateDoubleScalar(varValue->data.integer));
						} else if(varValue->type == SCARAB_OPAQUE){
							char *val = scarab_extract_string(varValue);
							mxSetField(codec_struct, c, buffer, 
									   mxCreateString(val));
						}
						
						free(buffer);
					}
				}      
			}
		}    
	}
	return codec_struct;
}

int insertDatumIntoEventList(mxArray *eventlist, const int index, ScarabDatum *datum){	
	long code = getScarabEventCode(datum);
	long long time = getScarabEventTime(datum);
	
	mxArray *data;
	
	data = getScarabEventData(datum);
	
	mxArray *old_code = mxGetField(eventlist, index, "event_code");
	if(old_code != NULL){
		mxDestroyArray(old_code);
	}
	mxSetField(eventlist, index, "event_code", mxCreateDoubleScalar((double)code));		
	
	mxArray *old_time = mxGetField(eventlist, index, "time_us");
	if(old_time != NULL){
		mxDestroyArray(old_time);
	}
	
	mxSetField(eventlist, index, "time_us", mxCreateDoubleScalar((double)time));
	
	mxArray *old_data = mxGetField(eventlist, index, "data");
	if(old_data){
		mxDestroyArray(old_data);
	}
	mxSetField(eventlist, index, "data", data);
	
	return code;
}	

int insertDatumIntoCodecList(mxArray *eventlist, const int index, ScarabDatum *datum){	
	long code = getScarabEventCode(datum);
	if(code == 0) {
		long long time = getScarabEventTime(datum);
		
		mxArray *data;
		
		data = getCodec(datum);
		
		mxArray *old_time = mxGetField(eventlist, index, "time_us");
		if(old_time != NULL){
			mxDestroyArray(old_time);
		}
		
		mxSetField(eventlist, index, "time_us", mxCreateDoubleScalar((double)time));
		
		mxArray *old_data = mxGetField(eventlist, index, "codec");
		if(old_data){
			mxDestroyArray(old_data);
		}
		mxSetField(eventlist, index, "codec", data);
	} else {
		//mexErrMsgTxt("Trying to insert a datum that isn't a codec");
	}
	
	return code;
}	

std::string getString(const mxArray *string_array_ptr) {
	// Allocate enough memory to hold the converted string.                     
	mwSize buflen = mxGetNumberOfElements(string_array_ptr) + 1;
	char *buf = (char *)mxCalloc(buflen, sizeof(char));
	
	// Copy the string data from string_array_ptr and place it into buf.        
	if (mxGetString(string_array_ptr, buf, buflen) != 0) {
		mxFree(buf);
		return std::string();
	}
	
	std::string new_string(buf);
	mxFree(buf);
	
	return new_string;
}


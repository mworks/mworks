/*
 *  MWorksMATLABTools.cpp
 *  MATLABTools
 *
 *  Created by David Cox on 12/20/06.
 *  Copyright 2006 MIT. All rights reserved.
 *
 */

#include "MWorksMATLABTools.h"

#include <stdio.h>
#include <string.h>
#include <vector>


static mxArray* getScarabInteger(ScarabDatum *datum);
static mxArray* getScarabDict(ScarabDatum *datum);
static mxArray* getScarabList(ScarabDatum *datum);
static mxArray* getScarabOpaque(ScarabDatum *datum);
static mxArray* getScarabFloat(ScarabDatum *datum);


mxArray* getScarabDatum(ScarabDatum *datum) {
    if (datum == NULL || datum->type == SCARAB_NULL) {
        // An empty array is the closest equivalent of NULL in MATLAB
        return mxCreateDoubleMatrix(0, 0, mxREAL);
    }
    
    switch (datum->type) {
        case SCARAB_INTEGER:
            return getScarabInteger(datum);
            
        case SCARAB_DICT:
            return getScarabDict(datum);
            
        case SCARAB_LIST:
            return getScarabList(datum);
            
        case SCARAB_OPAQUE:
            return getScarabOpaque(datum);
            
        default:
            return getScarabFloat(datum);
    }
}


static mxArray* getScarabInteger(ScarabDatum *datum) {
    mxArray *integer = mxCreateNumericMatrix(1, 1, mxINT64_CLASS, mxREAL);
    *((long long *)mxGetData(integer)) = datum->data.integer;
    return integer;
}


static mxArray* getScarabDict(ScarabDatum *datum){
	const int n = datum->data.dict->size;
    std::vector<ScarabDatum *> keys, values;
    
    // To simplify iteration, extract the non-NULL keys and corresponding values
    for (int i = 0; i < datum->data.dict->tablesize; i++) {
        if (datum->data.dict->keys[i]) {
            keys.push_back(datum->data.dict->keys[i]);
            values.push_back(datum->data.dict->values[i]);
        }
    }
	
	char **fields = (char **) calloc(n, sizeof(char *));
	
	int dictionaries = 0;
	int lists = 0;
	int floats = 0;
	int unknowns = 0;	
	
	// fix me here
	for (int i = 0; i < n; i++) {
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
	}
	
	mwSize dims = 1;
	mwSize ndims = 1;
	mxArray *struct_array = mxCreateStructArray(ndims, &dims, n, (const char **)fields);
	
	for (int i = 0; i < n; i++) {
		mxArray *mx_datum = getScarabDatum(values[i]);
        if (mx_datum != NULL) {
            mxArray *old_thing = mxGetField(struct_array, 0, fields[i]);
            if(old_thing){
                mxDestroyArray(old_thing);
            }
            mxSetField(struct_array, 0, fields[i], mx_datum);
        }
	}
	
	
	for (int i = 0; i < n; i++) {
		if(fields[i]) {
			free(fields[i]);
		}
	}
	free(fields);
	
	return struct_array;
}


static mxArray* getScarabList(ScarabDatum *datum) {
    const int n = datum->data.list->size;
    ScarabDatum **values = datum->data.list->values;
    
    mxArray *cell_matrix = mxCreateCellMatrix(1, n);
    
    for (int i = 0; i < n; i++) {
        mxArray *mx_datum = getScarabDatum(values[i]);
        if (mx_datum != NULL) {
            mxSetCell(cell_matrix, i, mx_datum);
        }
    }
    
    return cell_matrix;
}


static mxArray* getScarabOpaque(ScarabDatum *datum) {
    unsigned char *data = datum->data.opaque.data;
    mxArray *opaque;
    
    if (scarab_opaque_is_string(datum)) {
        opaque = mxCreateString((char *)data);
    } else {
        int size = datum->data.opaque.size;
        opaque = mxCreateNumericMatrix(1, size, mxUINT8_CLASS, mxREAL);
        if (opaque != NULL) {
            memcpy(mxGetData(opaque), data, size);
        }
    }
    
    return opaque;
}


static mxArray* getScarabFloat(ScarabDatum *datum) {
    double value;
    
    switch (datum->type) {
        case SCARAB_FLOAT:
            value = datum->data.floatp;
            break;
            
        case SCARAB_FLOAT_INF:
            value = mxGetInf();
            break;
            
        case SCARAB_FLOAT_NAN:
            value = mxGetNaN();
            break;
            
        case SCARAB_FLOAT_OPAQUE:
            value = scarab_extract_float(datum);
            break;
            
        default:
            value = 0.0;
            break;
    }
    
    return mxCreateDoubleScalar(value);
}


mxArray *getCodec(ScarabDatum *codec){
	int n_codec_entries = codec->data.dict->size;
    std::vector<ScarabDatum *> keys;
    
    // To simplify iteration, extract the non-NULL keys
    for (int i = 0; i < codec->data.dict->tablesize; i++) {
        if (codec->data.dict->keys[i]) {
            keys.push_back(codec->data.dict->keys[i]);
        }
    }
	
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
			int code = keys[c]->data.integer;
			ScarabDatum *serializedVar = scarab_dict_get(codec, keys[c]);
			
			mxSetField(codec_struct, c, "code", mxCreateDoubleScalar(code));
			
			if(serializedVar && serializedVar->type == SCARAB_DICT) {      
                ScarabDatum **varKeys = serializedVar->data.dict->keys;
                
				for(int d = 0; d < serializedVar->data.dict->tablesize; ++d) {
					if(varKeys[d]) {
						char *buffer = scarab_extract_string(varKeys[d]);
						
						ScarabDatum *varValue = scarab_dict_get(serializedVar, varKeys[d]);
						if(varValue == NULL){
							
						} else if(varValue->type == SCARAB_INTEGER){
							mxSetField(codec_struct, c, buffer, 
									   mxCreateDoubleScalar(varValue->data.integer));
						} else if(varValue->type == SCARAB_OPAQUE){
							char *val = scarab_extract_string(varValue);
							mxSetField(codec_struct, c, buffer, mxCreateString(val));
                            free(val);
						}
						
						free(buffer);
					}
				}      
			}
	}
	return codec_struct;
}


void insertEventIntoEventList(mxArray *eventlist, const int index, const MATLABEventInfo &event) {
    mxArray *old_code = mxGetField(eventlist, index, "event_code");
    if (old_code != NULL) {
        mxDestroyArray(old_code);
    }
    mxSetField(eventlist, index, "event_code", event.getCode());
    
    mxArray *old_time = mxGetField(eventlist, index, "time_us");
    if (old_time != NULL) {
        mxDestroyArray(old_time);
    }
    mxSetField(eventlist, index, "time_us", event.getTime());
    
    mxArray *old_data = mxGetField(eventlist, index, "data");
    if (old_data) {
        mxDestroyArray(old_data);
    }
    mxSetField(eventlist, index, "data", event.getData());
}


int insertDatumIntoCodecList(mxArray *eventlist, const int index, ScarabDatum *datum){	
	long code = DataFileUtilities::getScarabEventCode(datum);
	if(code == 0) {
		long long time = DataFileUtilities::getScarabEventTime(datum);
		
		mxArray *data;
		
		data = getCodec(DataFileUtilities::getScarabEventPayload(datum));
		
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

mxArray *createTopLevelCodecStruct(long ncodecs) {
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



























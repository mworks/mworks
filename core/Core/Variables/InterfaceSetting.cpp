/**
 * InterfaceSetting.cpp
 *
 * History:
 * David Cox on Tue Dec 10 2002 - Created.
 * Paul Jankunas on 01/27/05 - Fixed spacing added translation into scarab
 *                              object.
 *
 * Copyright (c) 2002 MIT. All rights reserved.
 */
 
#include "InterfaceSetting.h"
#include "Utilities.h"
#include <stdio.h>

InterfaceSetting::InterfaceSetting(Data * def, char * tag, char * full,
                                        char * desc, WhenType edit, 
                                        WhenType log, bool view, bool persist,
                                                    DomainType dType) {
    if((dType != M_DISCRETE_BOOLEAN) && (dType != M_INTEGER_INFINITE) 
                                     && (dType != M_CONTINUOUS_INFINITE)) {
        //merror(M_SYSTEM_MESSAGE_DOMAIN,
		mdebug("Incorrect constructor call for InterfaceSetting object");
    }
    tagname = copyString(tag);
    shortname = copyString(full);
    longname = copyString(desc);
    editable = edit;
	persistant = persist;
    logging = log;
    viewable = view;
    domain = dType;
    defaultvalue = Data(*def);
    
	
    //package = NULL;
    range = NULL;
    nvals = 0;
    parameter = NULL;
}

InterfaceSetting::InterfaceSetting(Data * def, char * tag, char * full,
                                    char * desc, WhenType edit, WhenType log,
                                    bool view,  bool persist, DomainType dType, 
											Data * rg) {
    if(dType != M_INTEGER_FINITE && dType != M_CONTINUOUS_FINITE) {
        merror(M_SYSTEM_MESSAGE_DOMAIN,
			"Incorrect call for non-finite domain type");
    }
    tagname = copyString(tag);
    shortname = copyString(full);
    longname = copyString(desc);
    editable = edit;
	persistant = persist;
    logging = log;
    viewable = view;
    domain = dType;
    defaultvalue = Data(*def);
    
    //package = NULL;
    range = new Data[2];
    nvals = 2;
    parameter = NULL;    
    if(rg != NULL) {
        range[0] = Data(rg[0]);
        range[1] = Data(rg[1]) ;
    }
}

InterfaceSetting::InterfaceSetting(Data * def, char * tag, char * full, 
                        char * desc, WhenType edit, WhenType log, bool view, 
                         bool persist, DomainType dType, Data * rg, int numvals) {
    if(dType != M_DISCRETE) {
        merror(M_SYSTEM_MESSAGE_DOMAIN,
				"Incorrect call for non-discrete domain type");
    }
    tagname = copyString(tag);
    shortname = copyString(full);
    longname = copyString(desc);
    editable = edit;
	persistant = persist;
    viewable = view;
    domain = dType;
    range = new Data[numvals];
    nvals = numvals;
    for(int i = 0; i < nvals; i++) {
        if(rg == NULL) {
            continue;
        }
        range[i] = Data(rg[i]);
    }
    logging = log;	
    defaultvalue = Data(*def);
    
    //package = NULL;
    parameter = NULL;
}

InterfaceSetting::InterfaceSetting(ScarabDatum * datum) {
    ScarabDatum *runner, *string_datum;
    if(datum->type != SCARAB_DICT) {
        //mwarning(M_NETWORK_MESSAGE_DOMAIN,
		mdebug("Failed to create an InterfaceSetting object: invalid scarab type");
		return;
	}
    
    
	//tagname
	string_datum = scarab_new_string("tagname");
    runner = scarab_dict_get(datum, string_datum);
	scarab_free_datum(string_datum);
	if(runner == NULL || runner->type != SCARAB_OPAQUE){
		mwarning(M_NETWORK_MESSAGE_DOMAIN,
			"Invalid tagname on variable received in event stream.");
		tagname = "<unknown>";
	} else {
		int sz = runner->data.opaque.size;
		tagname = new char[sz+1];
		strncpy(tagname, (const char *)runner->data.opaque.data, sz+1);
    }
	
	
    //shortname
	string_datum = scarab_new_string("shortname");
    runner = scarab_dict_get(datum, string_datum);
	scarab_free_datum(string_datum);
	
	if(runner == NULL || runner->type != SCARAB_OPAQUE){
		mwarning(M_NETWORK_MESSAGE_DOMAIN,
			"Invalid short name on variable (%s) received in event stream.",
			tagname);
		shortname = "<unknown>";
	} else {
		int sz = runner->data.opaque.size;
		shortname = new char[sz+1];
		strncpy(shortname, (const char *)runner->data.opaque.data, sz+1);
	}
		
    //longname
	string_datum = scarab_new_string("longname");
    runner = scarab_dict_get(datum, string_datum);
	scarab_free_datum(string_datum);
	if(runner == NULL || runner->type != SCARAB_OPAQUE){
		mwarning(M_NETWORK_MESSAGE_DOMAIN,
			"Invalid long name on variable (%s) received in event stream.",
			tagname);
		longname = "<unknown>";
	} else {
		int sz = runner->data.opaque.size;
		longname = new char[sz+1];
		strncpy(longname, (const char *)runner->data.opaque.data, sz+1);
	}
		
    //editable
	string_datum = scarab_new_string("editable");
    runner = scarab_dict_get(datum, string_datum);
	scarab_free_datum(string_datum);
	if(runner == NULL || runner->type != SCARAB_INTEGER){
		mwarning(M_NETWORK_MESSAGE_DOMAIN,
			"Invalid editable value on variable (%s) received in event stream.",
			tagname);
		editable = M_NEVER;
	} else {
		editable = (WhenType) runner->data.integer;
    }
	
    //domain
	string_datum = scarab_new_string("domain");
    runner = scarab_dict_get(datum, string_datum);
	scarab_free_datum(string_datum);
	if(runner == NULL || runner->type != SCARAB_INTEGER){
		mwarning(M_NETWORK_MESSAGE_DOMAIN,
			"Invalid domain on variable (%s) received in event stream.",
			tagname);
		 domain = (DomainType)0;  // TODO real value.
	} else {
		domain = (DomainType) runner->data.integer;
    }
	
	
    //viewable
	string_datum = scarab_new_string("viewable");
    runner = scarab_dict_get(datum, string_datum);
	scarab_free_datum(string_datum);
	if(runner == NULL || runner->type != SCARAB_INTEGER){
		mwarning(M_NETWORK_MESSAGE_DOMAIN,
			"Invalid viewable value on variable (%s) received in event stream.",
			tagname);
		 viewable = true;
	} else {
		viewable = (runner->data.integer?true:false);
    }
	
	//persistant
	string_datum = scarab_new_string("persistant");
    runner = scarab_dict_get(datum, string_datum);
	scarab_free_datum(string_datum);
	if(runner == NULL || runner->type != SCARAB_INTEGER){
		mwarning(M_NETWORK_MESSAGE_DOMAIN,
			"Invalid viewable value on variable (%s) received in event stream.",
			tagname);
		 persistant = false;
	} else {
		persistant = (runner->data.integer?true:false);
    }
	
    //nvals
	string_datum = scarab_new_string("nvals");
    runner = scarab_dict_get(datum, string_datum);
	scarab_free_datum(string_datum);
	if(runner == NULL || runner->type != SCARAB_INTEGER){
		mwarning(M_NETWORK_MESSAGE_DOMAIN,
			"Invalid # of values on variable (%s) received in event stream.",
			tagname);
		 nvals = 0;  // TODO real value.
	} else {
		nvals = runner->data.integer;
    }
	
    //range
    if(nvals == 0) {
        range = NULL;
    } else {
        range = new Data[nvals];
    }
    for(int i = 0; i < nvals; i++) {
        runner = scarab_dict_get(datum, scarab_new_integer(i));
		// TODO: check this
		range[i] = Data(runner);
    }
    
    //logging
	string_datum = scarab_new_string("logging");
    runner = scarab_dict_get(datum, string_datum);
	scarab_free_datum(string_datum);
	if(runner == NULL || runner->type != SCARAB_INTEGER){
		mwarning(M_NETWORK_MESSAGE_DOMAIN,
			"Invalid logging value on variable (%s) received in event stream.",
			tagname);
		 logging = (WhenType)M_WHEN_CHANGED;  // TODO real value.
	} else {
		logging = (WhenType) runner->data.integer;
    }
	
	
    // default value
	string_datum = scarab_new_string("defaultvalue");
    runner = scarab_dict_get(datum, string_datum);
	scarab_free_datum(string_datum);
	if(runner == NULL){
		mwarning(M_NETWORK_MESSAGE_DOMAIN,
			"Invalid default value on variable (%s) received in event stream.",
			tagname);
		 defaultvalue = Data(0L);
	} else {
		defaultvalue = Data(runner);
	}
	
    // parameter
    parameter = NULL;
    
    //scarab_memctx_free(memctx);
    //memctx = NULL;
    //package = NULL;
}

InterfaceSetting::InterfaceSetting(InterfaceSetting&  copy) {
    // sz + 1 adds a space for a null terminating character
    int sz = 0;
    // tagname
    sz = strlen(copy.getTagName());
    tagname = new char[sz + 1];
    strncpy(tagname, copy.getTagName(), sz+1);
    
    // shortname
    sz = strlen(copy.getShortName());
    shortname = new char[sz + 1];
    strncpy(shortname, copy.getShortName(), sz+1);
    
    // longname
    sz = strlen(copy.getLongName());
    longname = new char[sz +1];
    strncpy(longname, copy.getLongName(), sz+1);
    
    // editable
    editable = copy.getEditable();
    
    // domain
    domain = copy.getDomainType();
    
    // viewable
    viewable = copy.getViewable();
    
    // viewable
    persistant = copy.getPersistant();
    
    // nvals
    nvals = copy.getNumberValuesInRange();
    
    // range
    if(nvals == 0) { 
        range = NULL;
    } else {
        range = new Data[nvals];
        Data * cprange = copy.getRange();
        for(int i = 0; i < nvals; i++) {
            range[i] = cprange[i];
        }
    }
    
    // logging
    logging = copy.getLogging();
    
    // parameter
    parameter = copy.getVariable();  // don't need a deep copy here!
    
    // defaultvalue
    defaultvalue = Data(copy.getDefaultValue());
    memctx = NULL;
    //package = NULL;
}

InterfaceSetting::~InterfaceSetting() {
    if(tagname) {
        delete [] tagname;
        tagname = NULL;
    }
    if(shortname) {
        delete [] shortname;
        shortname = NULL;
    }
    if(longname) {
        delete [] longname;
        longname = NULL;
    }
    if(range) {
        delete [] range;
        range = NULL;
    }
    
    parameter = NULL;
    
    
    /*if(memctx) {
        scarab_memctx_free(memctx);
        memctx = NULL;
    }*/
}

Data * InterfaceSetting::getRange() {
    return range;
}

void InterfaceSetting::setVariable(Variable *newparam) {
    parameter = newparam;
}
		
Data InterfaceSetting::getDefaultValue() {
    return defaultvalue;
}
                
WhenType InterfaceSetting::getLogging() { 
    return logging;
}
		
char *InterfaceSetting::getTagName() {
    return tagname;
}
		
char *InterfaceSetting::getShortName() {
    return shortname;
}

char *InterfaceSetting::getLongName() {
    return longname;
}

WhenType InterfaceSetting::getEditable() {
    return editable;
}

DomainType InterfaceSetting::getDomainType() {
    return domain;
}

int InterfaceSetting::getNumberValuesInRange() {
    return nvals;
}

Variable *InterfaceSetting::getVariable() {
    return parameter;
}
                
bool InterfaceSetting::getViewable() {
    return viewable;
}

bool InterfaceSetting::getPersistant() {
    return persistant;
}

void InterfaceSetting::addRange(Data * val, int idx) {
    if(idx < 0 || idx >= nvals) {
        return;
    }
    range[idx] = Data(*val);
}

ScarabDatum * InterfaceSetting::getScarabDatum() {
    
	ScarabDatum *package = NULL;
	ScarabDatum *key = 0;
	ScarabDatum *value = 0;
	
    /*if(package != NULL) {
        return package;
    }*/
	
    // package size is 11 for every variable + nvals for the number of
    // items that are in range either 0, 2, or more
    package = scarab_dict_new(__PACKAGESIZE__ + nvals,
                                                    &scarab_dict_times2);
    //tagname
    key = scarab_new_string("tagname");
    value = scarab_new_string(tagname);
    scarab_dict_put(package, key, value);
    scarab_free_datum(key);
    scarab_free_datum(value);
                                        
    //shortname
    key = scarab_new_string("shortname");
    value = scarab_new_string(shortname);
    scarab_dict_put(package, key, value);
    scarab_free_datum(key);
    scarab_free_datum(value);
                                        
    //longname
    key = scarab_new_string("longname");
    value = scarab_new_string(longname);
    scarab_dict_put(package, key, value);
    scarab_free_datum(key);
    scarab_free_datum(value);
                                        
    //editable
    key = scarab_new_string("editable");
    value = scarab_new_integer(editable);
    scarab_dict_put(package, key, value);
    scarab_free_datum(key);
    scarab_free_datum(value);
                                        
    //domain
    key = scarab_new_string("domain");
    value = scarab_new_integer(domain);
    scarab_dict_put(package, key, value);
    scarab_free_datum(key);
    scarab_free_datum(value);
                                        
    //viewable
    key = scarab_new_string("viewable");
    value = scarab_new_integer((int)viewable);
    scarab_dict_put(package, key, value);
    scarab_free_datum(key);
    scarab_free_datum(value);
                                 
    //persistant
    key = scarab_new_string("persistant");
	value = scarab_new_integer((int)persistant);
    scarab_dict_put(package, key, value);
    scarab_free_datum(key);
    scarab_free_datum(value);
                                 
    //nvals
    key = scarab_new_string("nvals");
    value = scarab_new_integer(nvals);
    scarab_dict_put(package, key, value);
    scarab_free_datum(key);
    scarab_free_datum(value);
                                        
    //range
    if(range != NULL) {
        for(int i = 0; i < nvals; i++) {
			key = scarab_new_integer(i);
            scarab_dict_put(package, key, range[i].getScarabDatum());
			scarab_free_datum(key);
        }
    }

    //logging
    key = scarab_new_string("logging");
    value = scarab_new_integer(logging);
    scarab_dict_put(package, key, value);
    scarab_free_datum(key);
    scarab_free_datum(value);
                                        
    //defaultvalue
    key = scarab_new_string("defaultvalue");
    scarab_dict_put(package, key, defaultvalue.getScarabDatum());
    scarab_free_datum(key);

    return package;
}

void InterfaceSetting::printToSTDERR() {
    fprintf(stderr, "mInterfaceSetting.tagname => %s\n", tagname);
    fprintf(stderr, "mInterfaceSetting.shortname => %s\n", shortname);
    fprintf(stderr, "mInterfaceSetting.longname => %s\n", longname);
    fprintf(stderr, "mInterfaceSetting.editable => %d\n", editable);
    fprintf(stderr, "mInterfaceSetting.domain => %d\n", domain);
    fprintf(stderr, "mInterfaceSetting.viewable => %d\n", viewable);
    fprintf(stderr, "mInterfaceSetting.persistant => %d\n", persistant);
    fprintf(stderr, "mInterfaceSetting.logging => %d\n", logging);
    fprintf(stderr, "mInterfaceSetting.nvals => %d\n", nvals);
    if(nvals != 0) {
        for(int i = 0; i < nvals; i++) {
            fprintf(stderr, "mInterfaceSetting.range[%d] => %p\n", i, &range[i]);
            range[i].printToSTDERR();
        }
    }
    fprintf(stderr, "mInterfaceSetting.defaultvalue => %p\n", &defaultvalue);
    defaultvalue.printToSTDERR();
    fprintf(stderr, "mInterfaceSetting.parameter => %p\n", parameter);
    fflush(stderr);
}

/**
 *          PRIVATE FUNCTIONS
 */
char *InterfaceSetting::copyString(char *str) {
    int len = strlen(str);
    char * returnstr = new char[len+1];
    // the len+1 makes sure the string is NULL terminated
    strncpy(returnstr, str, len+1);
    return returnstr;
}
//mInterfaceSetting::InterfaceSetting(Data _defaultvalue, char *_tagname, 
//                    char *_shortname, char *_longname, WhenType _editable,
//                    bool _viewable, DomainType _domain, WhenType _logging) {
//							
//    if(_domain != M_DISCRETE_BOOLEAN && _domain != M_INTEGRAL_INFINITE && 
//                                        _domain != M_CONTINUOUS_INFINITE) {
//        throw "Incorrect call for non-boolean domain type";
//    }
//
//    tagname = copyString((char*)_tagname);
//    shortname = copyString((char*)_shortname);
//    longname = copyString((char*)_longname);
//    editable = _editable;
//    viewable = _viewable;
//    domain = _domain;
//    logging = _logging;
//    defaultvalue = _defaultvalue;
//    memctx = NULL;
//    package = NULL;
//    range = NULL;
//    nvals = 0;
//}
//
//mInterfaceSetting::InterfaceSetting(Data _defaultvalue, char *_tagname, 
//                    char *_shortname, char *_longname, WhenType _editable, 
//                    bool _viewable, DomainType _domain, Data * _range,
//                                                        WhenType _logging) {
//
//    if(_domain != M_INTEGRAL_FINITE && _domain != M_CONTINUOUS_FINITE) {
//        throw "Incorrect call for non-finite domain type";
//    }
//    tagname = copyString((char*)_tagname);
//    shortname = copyString((char*)_shortname);
//    longname = copyString((char*)_longname);
//    editable = _editable;
//    viewable = _viewable;
//    domain = _domain;
//    range = new Data[2];
//    nvals = 2;
//    range[0] = _range[0];
//    range[1] = _range[1];
//    logging = _logging;	
//    defaultvalue = _defaultvalue;
//    memctx = NULL;
//    package = NULL;
//}
//
//mInterfaceSetting::InterfaceSetting(Data _defaultvalue, char *_tagname, 
//                    char *_shortname, char *_longname, WhenType _editable, 
//                    bool _viewable, DomainType _domain, Data * _range,
//                                            int _nvals, WhenType _logging) {
//    if(_domain != M_DISCRETE) {
//        throw "Incorrect call for non-discrete domain type";
//    }
//    tagname = copyString((char *)_tagname);
//    shortname = copyString((char *)_shortname);
//    longname = copyString((char *)_longname);
//    editable = _editable;
//    viewable = _viewable;
//    domain = _domain;
//    range = new Data[_nvals];
//    nvals = _nvals;
//    for(int i = 0; i < nvals; i++) {
//        range[i] = _range[i];
//    }
//    logging = _logging;	
//    defaultvalue = _defaultvalue;
//    memctx = NULL;
//    package = NULL;
//}

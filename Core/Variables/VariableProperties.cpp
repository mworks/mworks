/**
* VariableProperties.cpp
 *
 * History:
 * David Cox on Tue Dec 10 2002 - Created.
 * Paul Jankunas on 01/27/05 - Fixed spacing added translation into scarab
 *                              object.
 *
 * Copyright (c) 2002 MIT. All rights reserved.
 */

#include "VariableProperties.h"
#include "Utilities.h"
#include <stdio.h>

#include <sstream>
using namespace mw;



VariableProperties::VariableProperties(Data * def, 
										 std::string tag, 
										 std::string full,
										 std::string desc, 
										 WhenType edit, 
										 WhenType log, 
										 bool view, 
										 bool persist,
										 DomainType dType, 
										 std::string _groups) {
    if((dType != M_DISCRETE_BOOLEAN) && (dType != M_INTEGER_INFINITE) 
	   && (dType != M_CONTINUOUS_INFINITE)) {
        //merror(M_SYSTEM_MESSAGE_DOMAIN,
		mdebug("Incorrect constructor call for VariableProperties object");
    }
    tagname = tag;
    shortname = full;
    longname = desc;
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
	
	groups = parseGroupList(_groups);
}

VariableProperties::VariableProperties(Data * def, 
										 std::string tag, 
										 std::string full,
										 std::string desc, 
										 WhenType edit, 
										 WhenType log,
										 bool view,  
										 bool persist, 
										 DomainType dType, 
										 Data * rg,
										 std::string _groups) {
    if(dType != M_INTEGER_FINITE && dType != M_CONTINUOUS_FINITE) {
        merror(M_SYSTEM_MESSAGE_DOMAIN,
			   "Incorrect call for non-finite domain type");
    }
    tagname = tag;
    shortname = full;
    longname = desc;
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
	
	groups = parseGroupList(_groups);
}

VariableProperties::VariableProperties(Data * def, 
										 std::string tag, 
										 std::string full, 
										 std::string desc, 
										 WhenType edit, 
										 WhenType log, 
										 bool view, 
										 bool persist, 
										 DomainType dType, 
										 Data * rg, 
										 int numvals, 
										 std::string _groups) {
    if(dType != M_DISCRETE) {
        merror(M_SYSTEM_MESSAGE_DOMAIN,
			   "Incorrect call for non-discrete domain type");
    }
    tagname = tag;
    shortname = full;
    longname = desc;
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
	
	groups = parseGroupList(_groups);
}

VariableProperties::VariableProperties(ScarabDatum * datum) {
    ScarabDatum *runner, *string_datum;
    if(datum->type != SCARAB_DICT) {
        //mwarning(M_NETWORK_MESSAGE_DOMAIN,
		mdebug("Failed to create an VariableProperties object: invalid scarab type");
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
		tagname.assign((const char *)runner->data.opaque.data);
    }
	
	
    //shortname
	string_datum = scarab_new_string("shortname");
    runner = scarab_dict_get(datum, string_datum);
	scarab_free_datum(string_datum);
	
	if(runner == NULL || runner->type != SCARAB_OPAQUE){
		mwarning(M_NETWORK_MESSAGE_DOMAIN,
				 "Invalid short name on variable (%s) received in event stream.",
				 tagname.c_str());
		shortname = "<unknown>";
	} else {
		shortname.assign((const char *)runner->data.opaque.data);
	}
	
    //longname
	string_datum = scarab_new_string("longname");
    runner = scarab_dict_get(datum, string_datum);
	scarab_free_datum(string_datum);
	if(runner == NULL || runner->type != SCARAB_OPAQUE){
		mwarning(M_NETWORK_MESSAGE_DOMAIN,
				 "Invalid long name on variable (%s) received in event stream.",
				 tagname.c_str());
		longname = "<unknown>";
	} else {
		longname.assign((const char *)runner->data.opaque.data);
	}
	
    //editable
	string_datum = scarab_new_string("editable");
    runner = scarab_dict_get(datum, string_datum);
	scarab_free_datum(string_datum);
	if(runner == NULL || runner->type != SCARAB_INTEGER){
		mwarning(M_NETWORK_MESSAGE_DOMAIN,
				 "Invalid editable value on variable (%s) received in event stream.",
				 tagname.c_str());
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
				 tagname.c_str());
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
				 tagname.c_str());
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
				 tagname.c_str());
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
				 tagname.c_str());
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
				 tagname.c_str());
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
				 tagname.c_str());
		defaultvalue = Data(0L);
	} else {
		defaultvalue = Data(runner);
	}
	
    // default value
	groups.clear();
	
	string_datum = scarab_new_string("groups");
    runner = scarab_dict_get(datum, string_datum);
	scarab_free_datum(string_datum);
	if(runner != NULL) {
		if(runner->type != SCARAB_LIST){
			mwarning(M_NETWORK_MESSAGE_DOMAIN,
					 "Invalid default value on variable (%s) received in event stream.",
					 tagname.c_str());
			
		} else {
			for(int i=0; i<runner->data.list->size; ++i) {
				ScarabDatum *group = runner->data.list->values[i];
				if(group != NULL && group->type == SCARAB_OPAQUE) {
					std::string gn((const char *)group->data.opaque.data);
					groups.push_back(gn);
				}
			}
		}
	}
	
    
	// parameter
    parameter = NULL;
    
    //scarab_memctx_free(memctx);
    //memctx = NULL;
    //package = NULL;
}

VariableProperties::VariableProperties(VariableProperties&  copy) {
    
	tagname = copy.getTagName();
	shortname = copy.getShortName();
	longname = copy.getLongName();
	
    
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
	
	groups = copy.getGroups();
	
}

VariableProperties::~VariableProperties() {
    
    if(range) {
        delete [] range;
        range = NULL;
    }
    
    parameter = NULL;
    
}

Data * VariableProperties::getRange() {
    return range;
}

void VariableProperties::setVariable(Variable *newparam) {
    parameter = newparam;
}

Data VariableProperties::getDefaultValue() {
    return defaultvalue;
}

WhenType VariableProperties::getLogging() { 
    return logging;
}

std::string VariableProperties::getTagName() {
    return tagname;
}

std::string VariableProperties::getShortName() {
    return shortname;
}

std::string VariableProperties::getLongName() {
    return longname;
}

WhenType VariableProperties::getEditable() {
    return editable;
}

DomainType VariableProperties::getDomainType() {
    return domain;
}

int VariableProperties::getNumberValuesInRange() {
    return nvals;
}

Variable *VariableProperties::getVariable() {
    return parameter;
}

bool VariableProperties::getViewable() {
    return viewable;
}

bool VariableProperties::getPersistant() {
    return persistant;
}

void VariableProperties::addRange(Data * val, int idx) {
    if(idx < 0 || idx >= nvals) {
        return;
    }
    range[idx] = Data(*val);
}

std::vector <std::string> VariableProperties::getGroups() {
	return groups;
}

VariableProperties::operator Data(){
	
	Data dict(M_DICTIONARY, 11);
	
	dict.addElement("tagname", tagname.c_str());
	dict.addElement("shortname", shortname.c_str());
	dict.addElement("longname", longname.c_str());
	dict.addElement("editable", Data((long)editable));
	dict.addElement("domain", Data((long)domain));
	dict.addElement("viewable", Data((long)viewable));
	dict.addElement("persistant", Data((long)persistant));
	dict.addElement("nvals", (long)nvals);
	dict.addElement("logging", Data((long)logging));
	
	if(!defaultvalue.isUndefined()) {
		dict.addElement("defaultvalue", defaultvalue);
	} else {
		dict.addElement("defaultvalue", 0L);
	}		
	
	if(groups.size() > 0) {
		Data gps(M_LIST, (int)groups.size());
		std::vector<std::string>::iterator iter = groups.begin();
		while (iter != groups.end()) {
			Data group(*iter);
			gps.addElement(group);
			++iter;
		}
		
		dict.addElement("groups", gps);
	}
	return dict;
}

ScarabDatum * VariableProperties::toScarabDatum() {
    
	// for now
	Data dict = this->operator Data();
	return dict.getScarabDatumCopy();
	
	
	/*	ScarabDatum *package = NULL;
	ScarabDatum *key = 0;
	ScarabDatum *value = 0;
	
    
	
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
	
    return package;*/
}

void VariableProperties::printToSTDERR() {
    fprintf(stderr, "mVariableProperties.tagname => %s\n", tagname.c_str());
    fprintf(stderr, "mVariableProperties.shortname => %s\n", shortname.c_str());
    fprintf(stderr, "mVariableProperties.longname => %s\n", longname.c_str());
    fprintf(stderr, "mVariableProperties.editable => %d\n", editable);
    fprintf(stderr, "mVariableProperties.domain => %d\n", domain);
    fprintf(stderr, "mVariableProperties.viewable => %d\n", viewable);
    fprintf(stderr, "mVariableProperties.persistant => %d\n", persistant);
    fprintf(stderr, "mVariableProperties.logging => %d\n", logging);
    fprintf(stderr, "mVariableProperties.nvals => %d\n", nvals);
    if(nvals != 0) {
        for(int i = 0; i < nvals; i++) {
            fprintf(stderr, "mVariableProperties.range[%d] => %p\n", i, &range[i]);
            range[i].printToSTDERR();
        }
    }
    fprintf(stderr, "mVariableProperties.defaultvalue => %p\n", &defaultvalue);
    defaultvalue.printToSTDERR();
    fprintf(stderr, "mVariableProperties.parameter => %p\n", parameter);
    fflush(stderr);
}

std::vector <std::string> VariableProperties::parseGroupList(const std::string &groups_csv) const {
	using namespace std;
	stringstream groupStream(groups_csv);
	string field;
	
	
	vector <string> gps;

	gps.push_back(std::string("# ALL VARIABLES"));
	
	while (getline(groupStream, field, ',')) {
		string::size_type start = field.find_first_not_of(" \t\v");
		if(start != string::npos) {
			string::size_type end = field.find_last_not_of(" \t\v");
			string trimmed(field.substr(start, end-start+1));
			if(trimmed.length() > 0) {
				gps.push_back(trimmed);
			}
		}
	}	
	
	return gps;
}



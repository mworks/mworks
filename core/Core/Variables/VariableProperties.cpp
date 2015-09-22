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
#include <boost/algorithm/string/trim.hpp>
#include <boost/tokenizer.hpp>


BEGIN_NAMESPACE_MW


VariableProperties::VariableProperties(Datum * def, 
										 std::string tag, 
										 std::string full,
										 std::string desc, 
										 WhenType edit, 
										 WhenType log, 
										 bool view, 
										 bool persist,
										 DomainType dType, 
										 std::string _groups,
                                         bool exclude)
{
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
    excludeFromDataFile = exclude;
    logging = log;
    viewable = view;
    domain = dType;
    defaultvalue = Datum(*def);
    
	
    //package = NULL;
    range = NULL;
    nvals = 0;
    parameter = NULL;
	
	groups = parseGroupList(_groups);
}

VariableProperties::VariableProperties(Datum * def, 
										 std::string tag, 
										 std::string full,
										 std::string desc, 
										 WhenType edit, 
										 WhenType log,
										 bool view,  
										 bool persist, 
										 DomainType dType, 
										 Datum * rg,
										 std::string _groups,
                                         bool exclude)
{
    if(dType != M_INTEGER_FINITE && dType != M_CONTINUOUS_FINITE) {
        merror(M_SYSTEM_MESSAGE_DOMAIN,
			   "Incorrect call for non-finite domain type");
    }
    tagname = tag;
    shortname = full;
    longname = desc;
    editable = edit;
	persistant = persist;
    excludeFromDataFile = exclude;
    logging = log;
    viewable = view;
    domain = dType;
    defaultvalue = Datum(*def);
    
    //package = NULL;
    range = new Datum[2];
    nvals = 2;
    parameter = NULL;    
    if(rg != NULL) {
        range[0] = Datum(rg[0]);
        range[1] = Datum(rg[1]) ;
    }
	
	groups = parseGroupList(_groups);
}

VariableProperties::VariableProperties(Datum * def, 
										 std::string tag, 
										 std::string full, 
										 std::string desc, 
										 WhenType edit, 
										 WhenType log, 
										 bool view, 
										 bool persist, 
										 DomainType dType, 
										 Datum * rg, 
										 int numvals, 
										 std::string _groups,
                                         bool exclude)
{
    if(dType != M_DISCRETE) {
        merror(M_SYSTEM_MESSAGE_DOMAIN,
			   "Incorrect call for non-discrete domain type");
    }
    tagname = tag;
    shortname = full;
    longname = desc;
    editable = edit;
	persistant = persist;
    excludeFromDataFile = exclude;
    viewable = view;
    domain = dType;
    range = new Datum[numvals];
    nvals = numvals;
    for(int i = 0; i < nvals; i++) {
        if(rg == NULL) {
            continue;
        }
        range[i] = Datum(rg[i]);
    }
    logging = log;	
    defaultvalue = Datum(*def);
    
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
				 "Invalid persistant value on variable (%s) received in event stream.",
				 tagname.c_str());
		persistant = false;
	} else {
		persistant = (runner->data.integer?true:false);
    }
    
    //excludeFromDataFile
    string_datum = scarab_new_string("exclude_from_data_file");
    runner = scarab_dict_get(datum, string_datum);
    scarab_free_datum(string_datum);
    if(runner == NULL || runner->type != SCARAB_INTEGER){
        mwarning(M_NETWORK_MESSAGE_DOMAIN,
                 "Invalid exclude_from_data_file value on variable (%s) received in event stream.",
                 tagname.c_str());
        excludeFromDataFile = false;
    } else {
        excludeFromDataFile = (runner->data.integer?true:false);
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
        range = new Datum[nvals];
    }
    for(int i = 0; i < nvals; i++) {
        runner = scarab_dict_get(datum, scarab_new_integer(i));
		// TODO: check this
		range[i] = Datum(runner);
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
		defaultvalue = Datum(0L);
	} else {
		defaultvalue = Datum(runner);
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
    
    // persistant
    persistant = copy.getPersistant();
    
    // excludeFromDataFile
    excludeFromDataFile = copy.getExcludeFromDataFile();
    
    // nvals
    nvals = copy.getNumberValuesInRange();
    
    // range
    if(nvals == 0) { 
        range = NULL;
    } else {
        range = new Datum[nvals];
        Datum * cprange = copy.getRange();
        for(int i = 0; i < nvals; i++) {
            range[i] = cprange[i];
        }
    }
    
    // logging
    logging = copy.getLogging();
    
    // parameter
    parameter = copy.getVariable();  // don't need a deep copy here!
    
    // defaultvalue
    defaultvalue = Datum(copy.getDefaultValue());
	
	groups = copy.getGroups();
	
}

VariableProperties::~VariableProperties() {
    
    if(range) {
        delete [] range;
        range = NULL;
    }
    
    parameter = NULL;
    
}

Datum * VariableProperties::getRange() {
    return range;
}

void VariableProperties::setVariable(Variable *newparam) {
    parameter = newparam;
}

Datum VariableProperties::getDefaultValue() {
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

bool VariableProperties::getExcludeFromDataFile() {
    return excludeFromDataFile;
}

void VariableProperties::addRange(Datum * val, int idx) {
    if(idx < 0 || idx >= nvals) {
        return;
    }
    range[idx] = Datum(*val);
}

std::vector <std::string> VariableProperties::getGroups() {
	return groups;
}

VariableProperties::operator Datum(){
	
 Datum dict(M_DICTIONARY, 12);
	
	dict.addElement("tagname", tagname.c_str());
	dict.addElement("shortname", shortname.c_str());
	dict.addElement("longname", longname.c_str());
	dict.addElement("editable", Datum((long)editable));
	dict.addElement("domain", Datum((long)domain));
	dict.addElement("viewable", Datum((long)viewable));
	dict.addElement("persistant", Datum((long)persistant));
    dict.addElement("exclude_from_data_file", Datum((long)excludeFromDataFile));
	dict.addElement("nvals", (long)nvals);
	dict.addElement("logging", Datum((long)logging));
	
	if(!defaultvalue.isUndefined()) {
		dict.addElement("defaultvalue", defaultvalue);
	} else {
		dict.addElement("defaultvalue", 0L);
	}		
	
	if(groups.size() > 0) {
	 Datum gps(M_LIST, (int)groups.size());
		std::vector<std::string>::iterator iter = groups.begin();
		while (iter != groups.end()) {
		 Datum group(*iter);
			gps.addElement(group);
			++iter;
		}
		
		dict.addElement("groups", gps);
	}
	return dict;
}

ScarabDatum * VariableProperties::toScarabDatum() {
 Datum dict = this->operator Datum();
	return dict.getScarabDatumCopy();
}

void VariableProperties::printToSTDERR() {
    fprintf(stderr, "mVariableProperties.tagname => %s\n", tagname.c_str());
    fprintf(stderr, "mVariableProperties.shortname => %s\n", shortname.c_str());
    fprintf(stderr, "mVariableProperties.longname => %s\n", longname.c_str());
    fprintf(stderr, "mVariableProperties.editable => %d\n", editable);
    fprintf(stderr, "mVariableProperties.domain => %d\n", domain);
    fprintf(stderr, "mVariableProperties.viewable => %d\n", viewable);
    fprintf(stderr, "mVariableProperties.persistant => %d\n", persistant);
    fprintf(stderr, "mVariableProperties.excludeFromDataFile => %d\n", excludeFromDataFile);
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
    vector <string> gps;
    gps.push_back(std::string("# ALL VARIABLES"));
    
    for (string field : boost::tokenizer<boost::escaped_list_separator<char>>(groups_csv)) {
        boost::algorithm::trim(field);
        if (!field.empty()) {
            gps.push_back(field);
        }
    }
    
    return std::move(gps);
}


END_NAMESPACE_MW


























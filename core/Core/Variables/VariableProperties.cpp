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
    tagname = tag;
	persistant = persist;
    excludeFromDataFile = exclude;
    logging = log;
    defaultvalue = Datum(*def);
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

bool VariableProperties::getPersistant() {
    return persistant;
}

bool VariableProperties::getExcludeFromDataFile() {
    return excludeFromDataFile;
}

std::vector <std::string> VariableProperties::getGroups() {
	return groups;
}

VariableProperties::operator Datum(){
	
 Datum dict(M_DICTIONARY, 12);
	
	dict.addElement("tagname", tagname.c_str());
	dict.addElement("persistant", Datum((long)persistant));
    dict.addElement("exclude_from_data_file", Datum((long)excludeFromDataFile));
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


























/**
 * VariableProperties.cpp
 *
 * Copyright (c) 2002-2016 MIT. All rights reserved.
 */

#include "VariableProperties.h"
#include "Utilities.h"
#include <boost/algorithm/string/trim.hpp>
#include <boost/tokenizer.hpp>


BEGIN_NAMESPACE_MW


VariableProperties::VariableProperties(const Datum &def,
                                       const std::string &tag,
                                       WhenType log,
                                       bool persist,
                                       const std::string &groups,
                                       bool exclude) :
    tagname(tag),
    persistant(persist),
    excludeFromDataFile(exclude),
    logging(log),
    defaultvalue(def),
    groups(parseGroupList(groups))
{ }


VariableProperties::VariableProperties(const Datum &datum) {
    if (!datum.isDictionary()) {
        merror(M_NETWORK_MESSAGE_DOMAIN, "Failed to create an VariableProperties object: invalid Datum type");
        return;
    }
    
    // tagname
    {
        Datum d = datum.getElement("tagname");
        if (!d.isString()) {
            mwarning(M_NETWORK_MESSAGE_DOMAIN,
                     "Invalid tagname on variable received in event stream.");
            tagname = "<unknown>";
        } else {
            tagname = d.getString();
        }
    }
    
    // persistant
    {
        Datum d = datum.getElement("persistant");
        if (!d.isInteger()) {
            mwarning(M_NETWORK_MESSAGE_DOMAIN,
                     "Invalid persistant value on variable (%s) received in event stream.",
                     tagname.c_str());
            persistant = false;
        } else {
            persistant = d.getBool();
        }
    }
    
    // excludeFromDataFile
    {
        Datum d = datum.getElement("exclude_from_data_file");
        if (!d.isInteger()) {
            mwarning(M_NETWORK_MESSAGE_DOMAIN,
                     "Invalid exclude_from_data_file value on variable (%s) received in event stream.",
                     tagname.c_str());
            excludeFromDataFile = false;
        } else {
            excludeFromDataFile = d.getBool();
        }
    }
    
    // logging
    {
        Datum d = datum.getElement("logging");
        if (!d.isInteger()) {
            mwarning(M_NETWORK_MESSAGE_DOMAIN,
                     "Invalid logging value on variable (%s) received in event stream.",
                     tagname.c_str());
            logging = M_WHEN_CHANGED;
        } else {
            logging = (WhenType)d.getInteger();
        }
    }
    
    // default value
    {
        Datum d = datum.getElement("defaultvalue");
        if (d.isUndefined()) {
            mwarning(M_NETWORK_MESSAGE_DOMAIN,
                     "Invalid defaultvalue value on variable (%s) received in event stream.",
                     tagname.c_str());
            defaultvalue = Datum(0L);
        } else {
            defaultvalue = d;
        }
    }
    
    // groups
    {
        Datum d = datum.getElement("groups");
        if (!d.isUndefined()) {
            if (!d.isList()) {
                mwarning(M_NETWORK_MESSAGE_DOMAIN,
                         "Invalid groups value on variable (%s) received in event stream.",
                         tagname.c_str());
            } else {
                for (auto &group : d.getList()) {
                    if (group.isString()) {
                        groups.push_back(group.getString());
                    }
                }
            }
        }
    }
}


const Datum& VariableProperties::getDefaultValue() const {
    return defaultvalue;
}

WhenType VariableProperties::getLogging() const {
    return logging;
}

const std::string& VariableProperties::getTagName() const {
    return tagname;
}

bool VariableProperties::getPersistant() const {
    return persistant;
}

bool VariableProperties::getExcludeFromDataFile() const {
    return excludeFromDataFile;
}

const std::vector<std::string>& VariableProperties::getGroups() const {
	return groups;
}

VariableProperties::operator Datum() const {
	
 Datum dict(M_DICTIONARY, 6);
	
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
		auto iter = groups.begin();
		while (iter != groups.end()) {
		 Datum group(*iter);
			gps.addElement(group);
			++iter;
		}
		
		dict.addElement("groups", gps);
	}
	return dict;
}


std::vector <std::string> VariableProperties::parseGroupList(const std::string &groups_csv) const {
    vector <string> gps;
    gps.push_back(std::string(ALL_VARIABLES));
    
    for (string field : boost::tokenizer<boost::escaped_list_separator<char>>(groups_csv)) {
        boost::algorithm::trim(field);
        if (!field.empty()) {
            gps.push_back(field);
        }
    }
    
    return std::move(gps);
}


END_NAMESPACE_MW


























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


VariableProperties VariableProperties::fromDatum(const Datum &datum) {
    if (!datum.isDictionary()) {
        merror(M_NETWORK_MESSAGE_DOMAIN, "Failed to create an VariableProperties object: invalid Datum type");
        return VariableProperties();
    }
    
    // tagname
    std::string tagName;
    {
        Datum d = datum.getElement("tagname");
        if (!d.isString()) {
            mwarning(M_NETWORK_MESSAGE_DOMAIN,
                     "Invalid tagname on variable received in event stream.");
            tagName = "<unknown>";
        } else {
            tagName = d.getString();
        }
    }
    
    // persistent
    bool persistent = false;
    {
        Datum d = datum.getElement("persistent");
        if (!d.isInteger()) {
            mwarning(M_NETWORK_MESSAGE_DOMAIN,
                     "Invalid persistent value on variable (%s) received in event stream.",
                     tagName.c_str());
            persistent = false;
        } else {
            persistent = d.getBool();
        }
    }
    
    // excludeFromDataFile
    bool excludeFromDataFile = false;
    {
        Datum d = datum.getElement("exclude_from_data_file");
        if (!d.isInteger()) {
            mwarning(M_NETWORK_MESSAGE_DOMAIN,
                     "Invalid exclude_from_data_file value on variable (%s) received in event stream.",
                     tagName.c_str());
            excludeFromDataFile = false;
        } else {
            excludeFromDataFile = d.getBool();
        }
    }
    
    // logging
    WhenType logging = M_NEVER;
    {
        Datum d = datum.getElement("logging");
        if (!d.isInteger()) {
            mwarning(M_NETWORK_MESSAGE_DOMAIN,
                     "Invalid logging value on variable (%s) received in event stream.",
                     tagName.c_str());
            logging = M_WHEN_CHANGED;
        } else {
            logging = (WhenType)d.getInteger();
        }
    }
    
    // default value
    Datum defaultValue;
    {
        Datum d = datum.getElement("defaultvalue");
        if (d.isUndefined()) {
            mwarning(M_NETWORK_MESSAGE_DOMAIN,
                     "Invalid defaultvalue value on variable (%s) received in event stream.",
                     tagName.c_str());
            defaultValue = Datum(0L);
        } else {
            defaultValue = d;
        }
    }
    
    // groups
    std::vector<std::string> groups;
    {
        Datum d = datum.getElement("groups");
        if (!d.isUndefined()) {
            if (!d.isList()) {
                mwarning(M_NETWORK_MESSAGE_DOMAIN,
                         "Invalid groups value on variable (%s) received in event stream.",
                         tagName.c_str());
            } else {
                for (auto &group : d.getList()) {
                    if (group.isString()) {
                        groups.push_back(group.getString());
                    }
                }
            }
        }
    }
    
    return VariableProperties(defaultValue,
                              tagName,
                              logging,
                              persistent,
                              groups,
                              excludeFromDataFile);
}


Datum VariableProperties::toDatum() const {
	
 Datum dict(M_DICTIONARY, 6);
	
	dict.addElement("tagname", tagName.c_str());
	dict.addElement("persistent", Datum((long)persistent));
    dict.addElement("exclude_from_data_file", Datum((long)excludeFromDataFile));
	dict.addElement("logging", Datum((long)logging));
	
	if(!defaultValue.isUndefined()) {
		dict.addElement("defaultvalue", defaultValue);
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


std::vector<std::string> VariableProperties::parseGroupList(const std::string &groups_csv) {
    std::vector<std::string> gps;
    gps.push_back(std::string(ALL_VARIABLES));
    
    for (auto field : boost::tokenizer<boost::escaped_list_separator<char>>(groups_csv)) {
        boost::algorithm::trim(field);
        if (!field.empty()) {
            gps.push_back(field);
        }
    }
    
    return gps;
}


END_NAMESPACE_MW

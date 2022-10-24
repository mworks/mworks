/**
 * VariableProperties.h
 *
 * Copyright (c) 2002-2016 MIT. All rights reserved.
 */

#ifndef UI_INTERFACE_H
#define UI_INTERFACE_H
 
#include "GenericData.h"
#include "stdlib.h"
#include <string>
#include <vector>
#include <limits.h>
#include <float.h>


BEGIN_NAMESPACE_MW


#define ALL_VARIABLES "# ALL VARIABLES"


enum WhenType {
    M_NEVER = -1,
    //M_WHEN_IDLE,
    //M_ALWAYS,
    //M_AT_STARTUP,
    //M_EVERY_TRIAL,
    M_WHEN_CHANGED = 4  // Maintain legacy value, which is present in old data files
};


enum DomainType {
    M_CONTINUOUS_INFINITE,
    M_CONTINUOUS_FINITE,
    M_DISCRETE,
    M_DISCRETE_BOOLEAN,
    M_INTEGER_FINITE,
    M_INTEGER_INFINITE,
    M_STRUCTURED
};


class VariableProperties {
    
public:
    VariableProperties() :
        logging(M_NEVER),
        persistent(false),
        excludeFromDataFile(false)
    { }
    
    VariableProperties(const Datum &def,
                       const std::string &tag,
                       WhenType log,
                       bool persist,
                       const std::string &groups = "",
                       bool exclude = false,
                       const std::string &desc = "") :
        VariableProperties(def, tag, log, persist, parseGroupList(groups), exclude, desc)
    { }
    
    // This constructor exists only for compatibility with existing code.  New code
    // should use the preceding constructor.
    VariableProperties(const Datum &def,
                       const std::string &tag,
                       const std::string &full,
                       const std::string &desc,
                       WhenType edit,
                       WhenType log,
                       bool view,
                       bool persist,
                       DomainType dType,
                       const std::string &groups,
                       bool exclude = false) :
        VariableProperties(def, tag, log, persist, groups, exclude, desc)
    { }
    
    const std::string & getTagName() const { return tagName; }
    const Datum & getDefaultValue() const { return defaultValue; }
    WhenType getLogging() const { return logging; }
    bool getPersistent() const { return persistent; }
    bool getExcludeFromDataFile() const { return excludeFromDataFile; }
    const std::vector<std::string> & getGroups() const { return groups; }
    const std::string & getDescription() const { return description; }
    
    static VariableProperties fromDatum(const Datum &datum);
    Datum toDatum() const;
    
private:
    static std::vector<std::string> parseGroupList(const std::string &groups_csv);
    
    VariableProperties(const Datum &def,
                       const std::string &tag,
                       WhenType log,
                       bool persist,
                       const std::vector<std::string> &groups,
                       bool exclude,
                       const std::string &desc) :
        tagName(tag),
        defaultValue(def),
        logging(log),
        persistent(persist),
        excludeFromDataFile(exclude),
        groups(groups),
        description(desc)
    { }
    
    const std::string tagName;
    const Datum defaultValue;
    const WhenType logging;
    const bool persistent;
    const bool excludeFromDataFile;
    const std::vector<std::string> groups;
    const std::string description;
    
};


END_NAMESPACE_MW


#endif

/**
 * VariableProperties.h
 *
 * Copyright (c) 2002-2016 MIT. All rights reserved.
 */

#ifndef UI_INTERFACE_H
#define UI_INTERFACE_H
 
#include "GenericData.h"
#include "GenericVariable.h"
#include "stdlib.h"
#include <string>
#include <vector>
#include <limits.h>
#include <float.h>


BEGIN_NAMESPACE_MW


enum DomainType{ M_CONTINUOUS_INFINITE, M_CONTINUOUS_FINITE, M_DISCRETE,
                  M_DISCRETE_BOOLEAN, M_INTEGER_FINITE, M_INTEGER_INFINITE,
                  M_STRUCTURED };


class VariableProperties {
	private:
		std::string tagname; // element tag
		bool persistant; // save the variable from run to run
        bool excludeFromDataFile; // should the variable be excluded from data files
		WhenType logging; // when does this variable get logged
		Datum defaultvalue; // the default value Datum object.
		std::vector <std::string> groups; // the groups that the variable belongs to
    
		std::vector <std::string> parseGroupList(const std::string &groups_csv) const;

	public:
        VariableProperties(const Datum &def,
                           const std::string &tag,
                           WhenType log,
                           bool persist,
                           const std::string &groups = "",
                           bool exclude = false);

        // This constructor exists only for compatibility with existing code.  New code
        // should use the preceding constructor.
        VariableProperties(Datum * def,
							std::string tag, 
							std::string full, 
							std::string desc,
							WhenType edit, 
							WhenType log, 
							bool view, 
							bool persist,
							DomainType dType, 
							std::string groups,
                            bool exclude = false) :
            VariableProperties(*def, tag, log, persist, groups, exclude)
        { }

        explicit VariableProperties(const Datum &datum);
    
        const Datum& getDefaultValue() const;
		WhenType getLogging() const;
        const std::string& getTagName() const;
		bool getPersistant() const;
        bool getExcludeFromDataFile() const;
		const std::vector<std::string>& getGroups() const;
        
		/**
         * Packages this interface setting object into a dictionary.
         */
		operator Datum() const;
};


END_NAMESPACE_MW


#endif


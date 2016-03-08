/**
 * VariableProperties.h
 *
 * Description: 
 *
 * Note: When creating the ScarabDatum package the keys are the variable names
 * and the values are thier respective values.  This class still needs some
 * destructor methods.
 *
 * NOTE: The Param member of this object does not get sent across the network
 * because an Param object also has a reference to an VariableProperties.
 * There is a small memory leak still because if you use the setParameter
 * function the object becomes the owner of the Param, but the object is not
 * destroyed in the destrutor because of the copy constructor cannot deep
 * copy the Param object because it is NULL when adding VariableProperties
 * objects into the interface lists in the Parameter Registry.
 *
 * NOTE: VariableProperties() for objects of INFINITE and DISCRETE_BOOLEN
 * types that accept a pointer to an Datum object have an extra parameter 
 * called nothing in the argument list.  This is because the constructor has
 * the same method signature as the constructor that takes an Datum object.
 * Perl was unable to distinguish between the two objects and was calling
 * the wrong constructor.
 *
 * History:
 * David Cox on Tue Dec 10 2002 - Created.
 * Paul Jankunas on 01/27/05 - Fixed spacing added translation into scarab
 *                              object.
 * Paul Jankunas on 02/14/05 - Added copy constructor (broken).
 * Paul Jankunas on 4/5/05 - Fixing copy constructor.
 * Paul Jankunas on 06/03/05 - Added destructor, added constructors that take
 *      pointer objects of type Datum so that they can be called from perl.
 *
 * Copyright (c) 2002 MIT. All rights reserved.
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

        // Constructs an interface setting from a scarab object.
        // used in network communication
        VariableProperties(ScarabDatum * datum);
    
        const Datum& getDefaultValue() const;
		WhenType getLogging() const;
        const std::string& getTagName() const;
		bool getPersistant() const;
        bool getExcludeFromDataFile() const;
		const std::vector<std::string>& getGroups() const;
        
		/**
         * Packages this interface setting object into a dictionary.
         */
        ScarabDatum *toScarabDatum() const;
        
		operator Datum() const;
};


END_NAMESPACE_MW


#endif


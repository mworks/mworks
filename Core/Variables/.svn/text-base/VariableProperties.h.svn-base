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
 * types that accept a pointer to an Data object have an extra parameter 
 * called nothing in the argument list.  This is because the constructor has
 * the same method signature as the constructor that takes an Data object.
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
 *      pointer objects of type Data so that they can be called from perl.
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
namespace mw {
// TODO: wtf?
// number of items in pacakge
const int __PACKAGESIZE__ = 11;

enum DomainType{ M_CONTINUOUS_INFINITE, M_CONTINUOUS_FINITE, M_DISCRETE,
                  M_DISCRETE_BOOLEAN, M_INTEGER_FINITE, M_INTEGER_INFINITE,
                  M_STRUCTURED };

// These are used as error conditions when parsing the XML and also
// used for the bounds on infinite types.
const int M_INT_MIN = INT_MIN;
const int M_INT_MAX = INT_MAX;
const double M_FLOAT_MIN = DBL_MIN;
const double M_FLOAT_MAX = DBL_MAX;

// TODO: write a class destructor
class VariableProperties {
	protected:
		std::string tagname; // element tag
        std::string shortname; // short human readable name
        std::string longname; // a description.
		WhenType editable; // when can we edit the variable
		DomainType domain; // what kind of range of values
		bool viewable; // can the user see this variable
		bool persistant; // save the variable from run to run
        Data * range; // an array of values in a range
		int nvals; // number of values in the range.
		WhenType logging; // when does this variable get logged
		Variable *parameter; // the parameter object that this setting represents
		Data defaultvalue; // the default value Data object.
		std::vector <std::string> groups; // the groups that the variable belongs to
        
        
		// this is retarded and is an accident waiting to happen
		//ScarabDatum * package; // a scarab object for network comm.
    
		std::vector <std::string> parseGroupList(const std::string &groups_csv) const;
		
    private:
        // disable the assignment operator
        void operator=(const VariableProperties& that) { }

	public:
	
		VariableProperties(){ };
		
	
        /**
         * Constructors that use Data pointers instead of value arguments
         * so that they can be called from perl.  Call this constructor
         * to create DISCRETE_BOOLEAN and CONTINUOUS or INTEGRAL INFINITE
         * types of variables.
         */
        VariableProperties(Data * def, 
							std::string tag, 
							std::string full, 
							std::string desc,
							WhenType edit, 
							WhenType log, 
							bool view, 
							bool persist,
							DomainType dType, 
							std::string groups);

        /**
         * Constructors that use Data pointers instead of value arguments
         * so that they can be called from perl.  Call this constructor
         * to create CONTINUOUS or INTEGRAL FINITE types of variables.
         */
		VariableProperties(Data * def, 
							std::string tag, 
							std::string full, 
							std::string desc, 
							WhenType edit, 
							WhenType log, 
							bool view, 
							bool persist,
							DomainType dType, 
							Data * rg, 
							std::string groups);

        /**
         * Constructors that use Data pointers instead of value arguments
         * so that they can be called from perl.  Call this constructor
         * to create M_DISRETE types of variables.
         */
		VariableProperties(Data * def, 
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
							std::string groups);

        // Constructs an interface setting from a scarab object.
        // used in network communication
        VariableProperties(ScarabDatum * datum);
        
        //copy constructor
        VariableProperties(VariableProperties& copy);

        /**
         * Destructor.
         */
        virtual ~VariableProperties();
        
        Data getDefaultValue();
        Variable * getVariable();
		WhenType getLogging();
		WhenType getEditable();
        DomainType getDomainType();
        int getNumberValuesInRange();
        Data * getRange();
        std::string getTagName();
        std::string getShortName();
		std::string getLongName();
		bool getViewable();
		bool getPersistant();
        void setVariable(Variable *theparam);
        void addRange(Data *, int);
		std::vector <std::string> getGroups();
        
		/**
         * Packages this interface setting object into a dictionary.
         */
        virtual ScarabDatum *toScarabDatum();
        
		operator Data();
		
        /**
         * Prints its data members to stderr
         */
        void printToSTDERR();
};
}
#endif


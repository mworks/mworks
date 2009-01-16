/*
 *  XMLVariableWriter.h
 *  MonkeyWorksCore
 *
 *  Created by David Cox on 5/30/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */


#include <vector>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/shared_ptr.hpp>
#include "GenericVariable.h"

namespace mw {
	
	using namespace std;
	using namespace boost;
	
#include <libxml/parser.h>
#include <libxml/tree.h>
	
	class XMLVariableWriter {
		
    public:
		static void writeVariablesToFile(vector< shared_ptr<Variable> > variables, filesystem::path file);
		
		static xmlNodePtr variableToXML(shared_ptr<Variable> var);
		static xmlNodePtr recursiveValueToXML(const Data &value);    
	};
}

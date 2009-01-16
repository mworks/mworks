/*
 *  VariableSave.cpp
 *  MonkeyWorksCore
 *
 *  Created by bkennedy on 8/4/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#include <string>
//#include <sstream>
#include "VariableSave.h"
#include "Experiment.h"
#include "GenericData.h"
#include "ExpandableList.h"
#include "PlatformDependentServices.h"
#include "XMLVariableWriter.h"
//#include "EmbeddedPerlInterpreter.h"
using namespace mw;

bool VariableSave::saveExperimentwideVariables(const boost::filesystem::path &file) {
	using namespace std;

    ExpandableList<GlobalVariable> *variableList = 
		GlobalVariableRegistry->getGlobalVariables();

	int nelements = variableList->getNElements();
	
    vector< shared_ptr<Variable> > variables_to_write;
    
	for(int i=0; i< nelements; i++)
	{
		shared_ptr<Variable> var = (*variableList)[i];
		VariableProperties *interface = var->getProperties();
		
		if(interface->getPersistant()){
            variables_to_write.push_back(var);
        }
	}
    
    try {
        XMLVariableWriter::writeVariablesToFile(variables_to_write, file);
    } catch (std::exception& e){
        merror(M_SYSTEM_MESSAGE_DOMAIN, "Error writing variables to file");
        return false;
    }
	return true;
}
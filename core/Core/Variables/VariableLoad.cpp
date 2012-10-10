/*
 *  VariableLoad.cpp
 *  MWorksCore
 *
 *  Created by bkennedy on 10/26/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#include "VariableLoad.h"
#include "ComponentRegistry.h"
#include "XMLParser.h"
#include "Experiment.h"
#include "PlatformDependentServices.h"
#include <string>


BEGIN_NAMESPACE_MW


bool VariableLoad::loadExperimentwideVariables(const boost::filesystem::path &file) {
	
    shared_ptr<ComponentRegistry> reg = ComponentRegistry::getSharedRegistry();

    try {
        
        XMLParser parser(reg, file.string());
        parser.parse();
        
    } catch (SimpleException &e) {
        
        display_extended_error_information(e);
        return false;
        
    }
	
	return true;
    
/*	if(GlobalCurrentExperiment != 0) {
		EmbeddedPerlInterpreter interp;
		const std::string substitutionDescriptor("%s");
		std::string evalString("\\$xmlfile = qq(%s);");
		

		evalString.replace(evalString.find(substitutionDescriptor), 
						   substitutionDescriptor.length(), file.string());	
	  
		interp.evalString(evalString);

		interp.evalFile(prependScriptingPath("XMLSavedVarParser.pl").string());	
  
		return true;
	}

	return false;*/
}


END_NAMESPACE_MW

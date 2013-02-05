#ifndef UI_VARIABLE_LOAD_H
#define UI_VARIABLE_LOAD_H

/*
 *  VariableLoad.h
 *  MWorksCore
 *
 *  Created by bkennedy on 10/27/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 *  Load an XML file containing variable assignments
 *
 */

#include <boost/filesystem/path.hpp>

#include "MWorksMacros.h"

#define LOAD_VARIABLES_FILE "file"
#define LOAD_VARIABLES_FULL_PATH "full path"


BEGIN_NAMESPACE_MW


class VariableLoad {
	public:
		static bool loadExperimentwideVariables(const boost::filesystem::path &file);
};


END_NAMESPACE_MW


#endif


#ifndef UI_VARIABLE_SAVE_H
#define UI_VARIABLE_SAVE_H

/*
 *  VariableSave.h
 *  MWorksCore
 *
 *  Created by bkennedy on 8/4/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */


#include "boost/filesystem/path.hpp"

#include "MWorksMacros.h"

#define SAVE_VARIABLES_FILE "file"
#define SAVE_VARIABLES_FULL_PATH "full path"
#define SAVE_VARIABLES_OVERWRITE "overwrite"


BEGIN_NAMESPACE_MW


class VariableSave {
	public:
	static bool saveExperimentwideVariables(const boost::filesystem::path &file);
};


END_NAMESPACE_MW


#endif


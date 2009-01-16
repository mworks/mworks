#ifndef UI_VARIABLE_LOAD_H
#define UI_VARIABLE_LOAD_H

/*
 *  VariableLoad.h
 *  MonkeyWorksCore
 *
 *  Created by bkennedy on 10/27/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#include <boost/filesystem/path.hpp>

#define LOAD_VARIABLES_FILE "file"
#define LOAD_VARIABLES_FULL_PATH "full path"
namespace mw {
class VariableLoad {
	public:
		static bool loadExperimentwideVariables(const boost::filesystem::path &file);
};
}
#endif


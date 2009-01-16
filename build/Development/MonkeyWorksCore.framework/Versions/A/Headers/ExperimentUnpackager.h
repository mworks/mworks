/*
 *  ExperimentUnpackager.h
 *  MonkeyWorksCore
 *
 *  Created by Ben Kennedy on 1/5/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _EXPERIMENT_UNPACKAGER_H__
#define _EXPERIMENT_UNPACKAGER_H__


#include "GenericData.h"
#include <string>
#include "boost/filesystem/path.hpp"
namespace mw {
class ExperimentUnpackager {
public:
	bool unpackageExperiment(Data payload);
	boost::filesystem::path getUnpackagedExperimentPath();
	
private:
	boost::filesystem::path loadedExperimentFilename;
	Data payload;
	
	boost::filesystem::path 
		prependExperimentInstallPath(const std::string expname, 
									 const std::string experimentFilename);
	
	bool createFile(Data filename, Data buffer);
};
}
#endif



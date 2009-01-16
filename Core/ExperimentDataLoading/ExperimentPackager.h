/*!
 * @header ExperimentPackager.h
 *
 * @discussion Controls the packaging and unpackaging of experiment files and
 * all the media files that it references.
 *
 * History:<BR/>
 * paul on 1/25/06 - Created.<BR/>
 * 
 * @copyright MIT
 * @updated 1/25/06
 * @version 1.0.0
 */

#ifndef _EXPERIMENT_PACKAGER_H__
#define _EXPERIMENT_PACKAGER_H__


#include "GenericData.h"
#include <string>
#include "boost/filesystem/path.hpp"
#include "XMLParser.h"

#define M_PACKAGER_FILENAME_STRING "Filename"
#define M_PACKAGER_CONTENTS_STRING "Contents"
#define M_PACKAGER_EXPERIMENT_STRING "Experiment"
#define M_PACKAGER_MEDIA_BUFFERS_STRING "Media Buffers"

#define M_PACKAGER_FULL_NAME "full_path"
#define M_PACKAGER_RELATIVE_NAME "relative_path"

#define M_EXPERIMENT_PACKAGE_NUMBER_ELEMENTS_PER_UNIT 2
#define M_EXPERIMENT_PACKAGE_NUMBER_ELEMENTS 2

namespace mw {

class ExperimentPackager {
public:
	Data packageExperiment(const boost::filesystem::path 
							fileWithFullPath);
	
private:
	Data packageSingleFile(const boost::filesystem::path filepath, 
							const std::string filename);
};

class IncludedFilesParser : public XMLParser {

	protected:
	
		Data manifest;

	public:
	
		IncludedFilesParser(std::string _path):
									XMLParser(_path){
		
			manifest = Data(M_LIST, 1);
		}
		
		virtual ~IncludedFilesParser(){ }
	
		// instead of building experiment, just look for path arguments and save
		// the results
		virtual void _processCreateDirective(xmlNode *node){
		
			xmlNode *child = node->children;
			
			while(child != NULL){
			
				string name((const char *)child->name);
					
				if(name == "path"){
					string the_path((const char *)xmlNodeGetContent(child));
					manifest.addElement(the_path);
				}
				
				child = child->next;
			}
		}
		
		// don't do anything for these
		virtual void _processConnectDirective(xmlNode *node){ }
		virtual void _processAnonymousCreateDirective(xmlNode *node){ }
		virtual void _processInstanceDirective(xmlNode *node){ }
		virtual void _processFinalizeDirective(xmlNode *node){ }
		
		// let _processRangeReplicator() do its thing
		
		virtual Data getIncludedFilesManifest(){  return manifest; }
		
};
}

#endif

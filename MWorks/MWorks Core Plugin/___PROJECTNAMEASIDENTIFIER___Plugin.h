/*
 *  ___PROJECTNAME___Plugin.h
 *  MWPlugin
 *
 *  Created by ___FULLUSERNAME___ on ___DATE___.
 *  Copyright ___YEAR___ ___ORGANIZATIONNAME___. All rights reserved.
 *
 */
#ifndef ___PROJECTNAME____PLUGIN_H_
#define ___PROJECTNAME____PLUGIN_H_

#include <MWorksCore/Plugin.h>
using namespace mw;

extern "C"{
    Plugin *getPlugin();
}

class ___PROJECTNAME___Plugin : public Plugin {
    
	virtual void registerComponents(shared_ptr<mw::ComponentRegistry> registry);	
};


#endif
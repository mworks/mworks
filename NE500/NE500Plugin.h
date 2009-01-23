/*
 *  CoxlabCorePlugins.h
 *  CoxlabCorePlugins
 *
 *  Created by David Cox on 4/29/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include <MonkeyWorksCore/Plugin.h>
#include <boost/shared_ptr.hpp>
    

extern "C"{
    mw::Plugin *getPlugin();
}



class NE500Plugin : public mw::Plugin {
    
	virtual void registerComponents(boost::shared_ptr<mw::ComponentRegistry> registry);	
};


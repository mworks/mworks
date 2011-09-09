/*
 *  ___PROJECTNAMEASIDENTIFIER___Plugin.h
 *  ___PROJECTNAME___
 *
 *  Created by ___FULLUSERNAME___ on ___DATE___.
 *  Copyright ___YEAR___ ___ORGANIZATIONNAME___. All rights reserved.
 *
 */

#ifndef ___PROJECTNAMEASIDENTIFIER____PLUGIN_H_
#define ___PROJECTNAMEASIDENTIFIER____PLUGIN_H_

#include <MWorksCore/Plugin.h>

using namespace mw;


extern "C" {
    Plugin* getPlugin();
}


class ___PROJECTNAMEASIDENTIFIER___Plugin : public Plugin {
    virtual void registerComponents(shared_ptr<ComponentRegistry> registry);
};


#endif

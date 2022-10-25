/**
 * PluginManager.cpp
 *
 * Note: Some functions in this file are Cocoa dependent
 *
 * History:
 * David Cox on Fri Dec 27 2002 - Created.
 * Paul Jankunas on Wed Mar 23 2005 - Fixed spacing. Fixxed readPlugin
 *                  function so that it doesn't start its search from
 *                  the package directory.
 *
 * Copyright (c) 2002 MIT. All rights reserved.
 */

#include "PluginServices.h"
#include "ComponentRegistry.h"
#include "VariableRegistry.h"
#include "Plugin.h"
#include "PlatformDependentServices.h"
#include "EventBuffer.h"
#include "LoadingUtilities.h"
#include <boost/filesystem/operations.hpp>

#include <dlfcn.h>


BEGIN_NAMESPACE_MW


	bool registries_are_initialized = false;
	shared_ptr<ComponentRegistry> ComponentRegistry__;
	
	
	void initializeServiceRegistries(){
		
		global_variable_registry = shared_ptr<VariableRegistry>(new VariableRegistry(global_outgoing_event_buffer));
		
		registries_are_initialized = true;
	}
	
	
	void readPlugins(boost::filesystem::path dir_path){
		using namespace boost::filesystem;
		
		if ( !exists( dir_path ) ){ 
			return;
		}
		
		directory_iterator end_itr; // default construction yields past-the-end
		
		for ( directory_iterator itr( dir_path ); itr != end_itr; ++itr ){
			string plugin_name = itr->path().filename().string();
			string plugin_name_stripped = 
			plugin_name.substr(0, plugin_name.find_last_of("."));
			//cerr << "Loading " << plugin_name_stripped.c_str() << endl;
			readPlugin(plugin_name_stripped);
		}
		
	}
	
	
	void readPlugin(string path){
		
		// Typedef for the function pointer.
		typedef Plugin* (*GetPluginFunctionPtr)(); 
		GetPluginFunctionPtr getplug = NULL;	
		
		
		char dynamic_library_path[512]; // TODO define
		
#ifdef	__APPLE__
		
#if TARGET_OS_OSX
        snprintf(dynamic_library_path, sizeof(dynamic_library_path), "%s/%s.bundle/Contents/MacOS/%s",
                 pluginPath().string().c_str(), path.c_str(), path.c_str());
#elif TARGET_OS_IPHONE
        snprintf(dynamic_library_path, sizeof(dynamic_library_path), "%s/%s.framework/%s",
                 pluginPath().string().c_str(), path.c_str(), path.c_str());
        
        // On iOS, frameworks and plugins have the same extension and reside in the same
        // subdirectory of the app bundle.  However, frameworks are loaded automatically
        // at launch time, while plugins are loaded manually at run time.  Therefore, by
        // calling dlopen with mode RTLD_NOLOAD, we can determine if a given bundle
        // executable is already loaded and, hence, whether it's a framework or a
        // plugin.
        if (dlopen(dynamic_library_path, RTLD_LAZY | RTLD_NOLOAD)) {
            // This "plugin" is actually a framework, so take no further action.
            return;
        }
#else
#error  Unsupported platform
#endif
		
#elif	linux
        snprintf(dynamic_library_path, sizeof(dynamic_library_path), "%s/%s.so",
                 pluginPath().string().c_str(), path.c_str());
		
#endif
        
        mprintf("Loading %s", path.c_str());
        
		void *library_handle = dlopen(dynamic_library_path, RTLD_LAZY);
		
		if(library_handle == NULL){
			mwarning(M_PLUGIN_MESSAGE_DOMAIN, "Plugin module (%s) failed to load: %s", path.c_str(), dlerror());
			// TODO: throw an error?
			return;
		}
		
		getplug = (GetPluginFunctionPtr)dlsym(library_handle, "getPlugin");
		//dlclose(library_handle); 
		
		
		
        std::unique_ptr<Plugin> plugin;
        
		if (getplug) {
			plugin.reset(getplug());
		}
		
		if(!plugin){
			mwarning(M_PLUGIN_MESSAGE_DOMAIN,
					 "Plugin module (%s) failed to produce a valid plugin object", 
					 path.c_str());
			// TODO: throw an error?
			return;
		}
		
		shared_ptr<ComponentRegistry> component_registry = 
		ComponentRegistry::getSharedRegistry();
		
		// Now we've got a hold of the plugin and we can work with it
		plugin->registerComponents(component_registry);
		
	}


END_NAMESPACE_MW

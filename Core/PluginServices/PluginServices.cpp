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
#include "ComponentRegistry_new.h"
#include "VariableRegistry.h"
#include "Plugin.h"
#include "PlatformDependentServices.h"
#include "EventBuffer.h"
#include "LoadingUtilities.h"
#include <boost/filesystem/operations.hpp>

#ifdef	__APPLE__
#define USE_DLOPEN_DYNAMIC_LOADING	1
#endif

#if		__linux
#define USE_DLOPEN_DYNAMIC_LOADING  1
#endif

#ifdef	USE_DLOPEN_DYNAMIC_LOADING
#include <dlfcn.h>
#endif

#ifdef  USE_CORE_FOUNDATION_DYNAMIC_LOADING
#import <CoreFoundation/CoreFoundation.h>
#endif

namespace mw {
	bool registries_are_initialized = false;
	shared_ptr<mwComponentRegistry> mwComponentRegistry__;
	
	
	void initializeServiceRegistries(){
		
		GlobalVariableRegistry = shared_ptr<VariableRegistry>(new VariableRegistry(GlobalBufferManager));
		
		registries_are_initialized = true;
	}
	
	
	void readPlugins(boost::filesystem::path dir_path){
		using namespace boost::filesystem;
		
		if ( !exists( dir_path ) ){ 
			return;
		}
		
		directory_iterator end_itr; // default construction yields past-the-end
		
		for ( directory_iterator itr( dir_path ); itr != end_itr; ++itr ){
			string plugin_name = itr->filename();
			string plugin_name_stripped = 
			plugin_name.substr(0, plugin_name.find_last_of("."));
			cerr << "Loading " << plugin_name_stripped.c_str() << endl;
			mprintf("Loading %s", plugin_name_stripped.c_str());
			readPlugin(plugin_name_stripped);
		}
		
	}
	
	
	void readPlugin(string path){
		
		Plugin *plugin = NULL;
		
		// Typedef for the function pointer.
		typedef Plugin* (*GetPluginFunctionPtr)(); 
		GetPluginFunctionPtr getplug = NULL;	
		
		
#ifdef	USE_CORE_FOUNDATION_DYNAMIC_LOADING
		
		// don't prepend the path
		//path = prependPluginPath(path);
		
		// Make a CFURLRef from the CFString representation of the 
		// bundle's path. See the Core Foundation URL Services chapter 
		// for details.
		CFURLRef bundleURL = CFURLCreateWithFileSystemPath( 
														   kCFAllocatorDefault, CFStringCreateWithCString(NULL, path,
																										  kCFStringEncodingMacRoman), kCFURLPOSIXPathStyle, true );
		
		// Make a bundle instance using the URLRef.
		CFBundleRef myBundle = CFBundleCreate( kCFAllocatorDefault, bundleURL );
		
		if(myBundle == NULL){
			mwarning(M_PLUGIN_MESSAGE_DOMAIN,
					 "Plugin (%s) did not load properly", path);
			//throw an error?
			return;
		}
		
		// Try to load the executable from my bundle.
		didLoad = CFBundleLoadExecutable( myBundle );
		
		// If the code was successfully loaded, look for our function.
		if (didLoad) {
			// Now that the code is loaded, search for 
			// the function we want by name.
			getplug = ( GetPluginFunctionPtr )CFBundleGetFunctionPointerForName( 
																				myBundle, CFSTR("getPlugin") );
			
		}
		
		
#elif	USE_DLOPEN_DYNAMIC_LOADING
		
		char dynamic_library_path[512]; // TODO define
		
#ifdef	__APPLE__
		
		sprintf(dynamic_library_path, "%s/%s.bundle/Contents/MacOS/%s",
				pluginPath().string().c_str(), path.c_str(), path.c_str());
		
#elif	linux
		sprintf(dynamic_library_path, "%s/%s.so",
				pluginPath().string().c_str(), path.c_str());
		
#endif 
		
		void *library_handle = dlopen(dynamic_library_path, RTLD_LAZY);
		
		if(library_handle == NULL){
			mprintf("%s", dlerror());
		}
		
		getplug = (GetPluginFunctionPtr)dlsym(library_handle, "getPlugin");
		//dlclose(library_handle); 
		
#endif
		
		
		
		// If our function was found in the loaded code, 
		// call it with a test value.
		if (getplug) {
			// This should add 1 to whatever was passed in
			plugin = getplug();
		}
		
		if(!plugin){
			mwarning(M_PLUGIN_MESSAGE_DOMAIN,
					 "Plugin module (%s) failed to produce a valid plugin object", 
					 path.c_str());
			// TODO: throw an error?
			return;
		}
		
		shared_ptr<mwComponentRegistry> component_registry = 
		mwComponentRegistry::getSharedRegistry();
		
		// Now we've got a hold of the plugin and we can work with it
		plugin->registerComponents(component_registry);
		
	}
}

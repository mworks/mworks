#ifndef PLUGIN_SERVICES_H
#define PLUGIN_SERVICES_H

/**
 * PluginManager.h
 *
 * Description:
 * Utilities for loading in plugins
 *
 * Note: The functions in this file are Cocoa dependent.
 *
 * History:
 * David Cox on Fri Dec 27 2002 - Created.
 *
 * Copyright (c) 2002 MIT. All rights reserved.
 */

#include "ComponentRegistry_new.h"

#include <boost/filesystem/path.hpp>
namespace mw {
	extern bool registries_are_initialized;
	extern shared_ptr<mwComponentRegistry> mwComponentRegistry__;
	
	void initializeServiceRegistries();
	void readPlugins(boost::filesystem::path plugin_directory);
	/**
	 * Reads a Cocoa bundle from the absolute path 'path'.  At the moment
	 * it will only load Cocoa bundles and is therefore platform dependent.
	 */
	void readPlugin(string path);
	
	// Realtime Service Plugins
	void chooseDefaultScheduler();
	void chooseDefaultClock();
	void chooseDefaultStateSystem();
}
#endif
/**
 *  OpenGLContextManager.h
 *
 *  Created by David Cox on Thu Dec 05 2002.
 *  Copyright (c) 2002 MIT. All rights reserved.
 */

#ifndef OPENGL_CONTEXT_MANAGER__
#define OPENGL_CONTEXT_MANAGER__

#include "glew.h"
#include "RegisteredSingleton.h"
#include "Stimulus.h"


#include <Foundation/Foundation.h>
#include <Cocoa/Cocoa.h>
#include <IOKit/pwr_mgt/IOPMLib.h>



#import "MWorksCore/StimMirrorController.h"
#import "Scheduler.h"

#define M_FULLSCREEN_OPENGL_MODE 0
#define M_MIRRORED_OPENGL_MODE 1



// USE_COCOA_OPENGL
namespace mw {
	// A class to manage Cocoa OpenGL contexts
	class OpenGLContextManager : public Component{
		
    protected:
		
		NSMutableArray			*contexts;
        
        std::map<int, double>   display_refresh_rates;
		
		NSWindow				*mirror_window;
        NSOpenGLView            *mirror_view;
		BOOL					mirror_window_active;
        
        NSWindow                *fullscreen_window;
        NSOpenGLView            *fullscreen_view;
        BOOL                    fullscreen_window_active;
        
        IOPMAssertionID         display_sleep_block;
		
		int						main_display_index;
		
        NSScreen                *_getScreen(const int screen_number);
        CGDirectDisplayID       _getDisplayID(int screen_number);
        void                    _measureDisplayRefreshRate(int index);
        
        bool glew_initialized;
        
        void _initGlew(){
            
            if(glewInit() == GLEW_OK){
                glew_initialized = true;
            } else {
                glew_initialized = false;
            }
        }
        
    public:
        
		
        OpenGLContextManager();
        
        
        // Create a "mirror" window (smaller, movable window that displays whatever
        // is on the "main" display) and return its context index
        int newMirrorContext(bool sync_to_vbl);
                
        // Create a fullscreen context on a particular display
		int newFullscreenContext(int index);
        
        // "Let go" of captured fullscreen contexts
		void releaseDisplays();
		
        // Choose which display is going to have the "main" stimulus display
		void setMainDisplayIndex(int index) { main_display_index = index; }
		int getMainDisplayIndex() const { return main_display_index; }
        CGDirectDisplayID getMainDisplayID();
        CVReturn prepareDisplayLinkForMainDisplay(CVDisplayLinkRef displayLink);
        
        // Get information about a given monitor
		NSRect getDisplayFrame(const int index);
        double getDisplayRefreshRate(int index);
		int getDisplayWidth(const int index);
		int getDisplayHeight(const int index);
        
        NSOpenGLView *getFullscreenView(){  return fullscreen_view; }
		NSOpenGLView *getMirrorView(){  return mirror_view; }
        
		// Get information about the monitors attached to the system
        int getNMonitors();
        
        
        void setCurrent(int context_id);
                
        void updateAndFlush(int context_id){ flush(context_id, true); }
        void flush(int context_id, bool update=false);
        void flushCurrent();
		
        REGISTERED_SINGLETON_CODE_INJECTION(OpenGLContextManager)
        
	};
	
}

#endif  

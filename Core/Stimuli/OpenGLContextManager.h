/**
 *  OpenGLContextManager.h
 *
 *  Created by David Cox on Thu Dec 05 2002.
 *  Copyright (c) 2002 MIT. All rights reserved.
 */

#ifndef OPENGL_CONTEXT_MANAGER__
#define OPENGL_CONTEXT_MANAGER__

#include "RegisteredSingleton.h"

#include "Stimulus.h"

#include <Cocoa/Cocoa.h>

#import "MWorksCore/StimMirrorController.h"
#import "Scheduler.h"

#define M_FULLSCREEN_OPENGL_MODE 0
#define M_MIRRORED_OPENGL_MODE 1



// USE_COCOA_OPENGL
namespace mw {
	// A bastard class to manage Cocoa OpenGL contexts
	class OpenGLContextManager : public Component{
		
    protected:
		
		NSMutableArray			*contexts;
		
		NSWindow				*mirror_window;
        NSOpenGLView            *mirror_view;
		BOOL					mirror_window_active;
        
        NSWindow                *fullscreen_window;
        NSOpenGLView            *fullscreen_view;
        BOOL                    fullscreen_window_active;
		
		
		GLuint					synchronization_fence;
		shared_ptr<ScheduleTask>			beamNode;
		
		int						main_display_index;
		
        NSScreen                *_getScreen(const int screen_number);
        
    public:
		
        OpenGLContextManager();
        
        
        // Create a "mirror" window (smaller, movable window that displays whatever
        // is on the "main" display) and return its context index
        int newMirrorContext(int pixelDepth);
                
        // Create a fullscreen context on a particular display
		int newFullscreenContext(int pixelDepth, int index);
        
        // "Let go" of captured fullscreen contexts
		void releaseDisplays();
		
        // Choose which display is going to have the "main" stimulus display
		void setMainDisplayIndex(const int index);
		int getMainDisplayIndex() const;
        
        // Get information about a given monitor
		NSRect getDisplayFrame(const int index);
        int getDisplayRefreshRate(int index);
		int getDisplayWidth(const int index);
		int getDisplayHeight(const int index);
		
        
		// Get information about the monitors attached to the system
        int getNMonitors();
        
        
        void setCurrent(int context_id);
                
        void flush(int context_id);
        void flushCurrent();
		
		GLuint getFence(){  return synchronization_fence; }
		GLuint *getFencePointer(){ return &synchronization_fence; }
		
        REGISTERED_SINGLETON_CODE_INJECTION(OpenGLContextManager)
        
	};
	
	extern OpenGLContextManager *GlobalOpenGLContextManager;
}

#endif  

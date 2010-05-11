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
		NSMutableArray			*displayModes;
		NSDictionary			*currentDisplayMode;
		NSDictionary			*originalDisplayMode;
		
		NSMutableArray			*contexts;
		NSMutableArray			*screen_modes;
		NSMutableArray			*windows;
		NSMutableArray			*original_display_modes;
		NSMutableArray			*current_display_modes;
		
		NSWindow				*mirrorWindow;
		BOOL					mirrorWindowActive;
		
#define kMaxDisplays 100
		//LLDisplayCalibration	*displays;
		CGDirectDisplayID		display_ids[kMaxDisplays];
		int						n_monitors_available;
		NSOpenGLContext			*global_context;
		
		GLuint					synchronization_fence;
		shared_ptr<ScheduleTask>			beamNode;
		
		int						main_display_index;
		
    public:
		
        OpenGLContextManager();
        void queryDisplayModes();
        int createNewContext(int screen_mode);
        int newContext(int pixelDepth);
        int newMirrorContext(int pixelDepth);
        int newFullScreenContext(int pixelDepth);
		int newFullScreenContext(int pixelDepth, int index);
		void releaseDisplays();
		
		void setMainDisplayIndex(const int index);
		int getMainDisplayIndex() const;
		int getDisplayRefreshRate(int index);
		int getDisplayWidth(const int index);
		int getDisplayHeight(const int index);
		
		
		
        bool systemHasSecondMonitor();
		int getNMonitors();
        void initializeGlobalContext();
   
		NSOpenGLContext *getGlobalContext();
 		
        void setGlobalContextCurrent();
        void setCurrent(int context_id);
        void setContextToMirror(int context_id);
        void flush(int context_id);
        void flushCurrent();
		
		GLuint getFence(){  return synchronization_fence; }
		GLuint *getFencePointer(){ return &synchronization_fence; }
		
        REGISTERED_SINGLETON_CODE_INJECTION(OpenGLContextManager)
        
	};
	
	extern OpenGLContextManager *GlobalOpenGLContextManager;
}

#endif  

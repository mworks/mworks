/**
 *  OpenGLContextManager.h
 *
 *  Created by David Cox on Thu Dec 05 2002.
 *  Copyright (c) 2002 MIT. All rights reserved.
 */


/**
 *  NOTE: THIS IS A COMPLETE MESS AND NEEDS TO BE COMPLETELY CLEANED
 *  (AND POSSIBILY REDONE).  TREAD CAREFULLY
 */

#ifndef OPENGL_CONTEXT_MANAGER__
#define OPENGL_CONTEXT_MANAGER__

// THIS IS AN OS X SPECIFIC IMPLEMENTATION
#ifdef	__APPLE__	
#define		USE_COCOA_OPENGL	 1
#else
#undef		USE_COCOA_OPENGL	
#endif

// FOR NOW, IF !USE_COCOA_OPENGL, USE_GLUT_OPENGL
//#define USE_GLUT_OPENGL	1
//#define USE_SDL_OPENGL  1

#include "Stimulus.h"

#ifdef  USE_COCOA_OPENGL
#include <Cocoa/Cocoa.h>
//#import "Lablib/LLDisplayCalibration.h"

#import "MonkeyWorksCore/StimMirrorController.h"
#import "Scheduler.h"
#endif 
// USE_COCOA_OPENGL

#ifdef	USE_GLUT_OPENGL
#ifdef	__APPLE__
#include <GLUT/glut.h>
#elif	linux
#include <GL/glut.h>
#endif
#endif

#define M_FULLSCREEN_OPENGL_MODE 0
#define M_MIRRORED_OPENGL_MODE 1

#ifdef  USE_COCOA_OPENGL
//extern StimMirrorController *GlobalStimMirrorController;
#endif  
// USE_COCOA_OPENGL
namespace mw {
	// A bastard class to manage Cocoa OpenGL contexts
	class OpenGLContextManager {
		
    protected:
#ifdef	USE_COCOA_OPENGL
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
		
#endif		// ifdef USE_COCOA_OPENGL
		
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
        
#ifdef	USE_COCOA_OPENGL
		NSOpenGLContext *getGlobalContext();
#endif
		
        void setGlobalContextCurrent();
        void setCurrent(int context_id);
        void setContextToMirror(int context_id);
        void flush(int context_id);
        void flushCurrent();
		
		GLuint getFence(){  return synchronization_fence; }
		GLuint *getFencePointer(){ return &synchronization_fence; }
		
	};
	
	extern OpenGLContextManager *GlobalOpenGLContextManager;
}
#endif  

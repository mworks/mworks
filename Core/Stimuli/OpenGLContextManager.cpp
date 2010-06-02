/**
 * OpenGLContextManager.cpp
 *
 * Created by David Cox on Thu Dec 05 2002.
 * Copyright (c) 2002 MIT. All rights reserved.
 */

#include "OpenGLContextManager.h"
#include "StandardVariables.h"
#include "Experiment.h"
#include "Utilities.h"
#include "Event.h"
#include "ComponentRegistry.h"

using namespace mw;





//******************************************************************
//******************************************************************
//                      Using Cocoa OpenGL
//******************************************************************
// *****************************************************************


//#include "GLWindow.h"
#import <OpenGL/OpenGL.h>
#import <OpenGL/gl.h>
#import <OpenGL/glu.h>
#import <OpenGL/glext.h>
#import <IOKit/graphics/IOGraphicsTypes.h>
#import "StateSystem.h"

//#define kDefaultDisplay 1


// TODO: this may make more sense if it were in the stimDisplay class instead
//		 since multiple displays will have different VBLs etc. etc.
//		 This is currently just hacked in to show how it is done and to use
//		 in the meantime

#define M_BEAM_POSITION_CHECK_INTERVAL_US	2000
#define N_BEAM_POSITION_SUPPORT_CHECKS	50

struct beam_position_args {
	CGDirectDisplayID id;
	int display_height;
};

// TODO: it should be pretty easy to adapt this to spit out a genuine
//		(pre-time-stamped) VBL event that occurs pretty damned close to the
//		real VBL

void *announce_beam_position(void *arg){
	
	struct beam_position_args *beam_args = (struct beam_position_args *)arg; 
	
	
	double beam_percent = ((double)CGDisplayBeamPosition(beam_args->id)) /
	((double)beam_args->display_height);
	
	if(GlobalCurrentExperiment != NULL){
		
		shared_ptr <StateSystem> state_system = StateSystem::instance();
		
		if(state_system->isRunning()){
			beamPosition->setValue(Datum((double)beam_percent));
		}
	}
	
	return NULL;
}



OpenGLContextManager::OpenGLContextManager() {
	
	mirror_window_active = NO;
    fullscreen_window_active = NO;
    
    mirror_window = Nil;
    fullscreen_window = Nil;
    
    contexts = [[NSMutableArray alloc] init];
    
    has_fence = false;
    glew_initialized = false;
    
    main_display_index = 0;
}


int OpenGLContextManager::getNMonitors() {
	NSArray *screens = [NSScreen screens];
    if(screens != Nil){
        return [screens count];
    } else {
        return 0;
    }
}

NSScreen *OpenGLContextManager::_getScreen(const int index){
    NSArray *screens = [NSScreen screens];
    
    if(index < 0 || index > [screens count]){
        // TODO: better exception
        throw SimpleException("Attempt to query an invalid screen");
    }
    
    return [screens objectAtIndex: index];
}

NSRect OpenGLContextManager::getDisplayFrame(const int index){
    NSScreen *screen = _getScreen(index);
    NSRect frame = [screen frame];
    
    return frame;
}

int OpenGLContextManager::getDisplayWidth(const int index) {

    NSRect frame = getDisplayFrame(index);
    return frame.size.width;

}

int OpenGLContextManager::getDisplayHeight(const int index) {
    NSRect frame = getDisplayFrame(index);
    return frame.size.height;
}

int OpenGLContextManager::getDisplayRefreshRate(const int index){
    
    if(index < 0 || index > display_refresh_rates.size()){
        return 0;
    }
    
    double refresh_rate = display_refresh_rates[index];
        
    return (int)refresh_rate;
}

double OpenGLContextManager::_measureDisplayRefreshRate(const int index)
{
    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
    NSScreen *screen = _getScreen(index);
    
	NSDictionary *device_description = [screen deviceDescription];
    
    NSNumber *display_id_number = [device_description objectForKey:@"NSScreenNumber"];
    CGDirectDisplayID display_id = [display_id_number intValue];
    CGDisplayModeRef mode = CGDisplayCopyDisplayMode(display_id);
    [pool release];
    
    return CGDisplayModeGetRefreshRate(mode);
}



int OpenGLContextManager::newMirrorContext(int pixelDepth){
    
    // Determine the width and height of the mirror window

    double width = 100.0;    // conspicuously wrong defaults
	double height = 100.0;
	
    shared_ptr<ComponentRegistry> reg = ComponentRegistry::getSharedRegistry();
    shared_ptr<Variable> main_screen_info = reg->getVariable(MAIN_SCREEN_INFO_TAGNAME);
    
	if(main_screen_info != NULL){
        Datum info = *main_screen_info;
		
		if(info.hasKey(M_DISPLAY_WIDTH_KEY) 
		   && info.hasKey(M_DISPLAY_HEIGHT_KEY)
		   && info.hasKey(M_MIRROR_WINDOW_BASE_HEIGHT_KEY)){
			
			double display_width, display_height, display_aspect;
			display_width = info.getElement(M_DISPLAY_WIDTH_KEY);
			display_height = info.getElement(M_DISPLAY_HEIGHT_KEY);
			display_aspect = display_width / display_height;
			height = (double)info.getElement(M_MIRROR_WINDOW_BASE_HEIGHT_KEY);
			width = height * display_aspect;
		}
	}
    
    
    NSRect mirror_rect = NSMakeRect(50.0, 50.0, width, height);

    
    mirror_window = [[NSWindow alloc] initWithContentRect: mirror_rect styleMask:NSResizableWindowMask | NSMiniaturizableWindowMask backing:NSBackingStoreBuffered defer:YES];
        
    NSOpenGLPixelFormatAttribute attrs[] =
    {
        NSOpenGLPFADoubleBuffer,
        0
    };
    
    NSOpenGLPixelFormat* pixel_format = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
    
    NSOpenGLContext *opengl_context = [[NSOpenGLContext alloc] initWithFormat:pixel_format shareContext:Nil];
    
    NSRect view_rect = NSMakeRect(0.0, 0.0, mirror_rect.size.width, mirror_rect.size.height);
    
    mirror_view = [[NSOpenGLView alloc] initWithFrame:view_rect pixelFormat: pixel_format];
    [mirror_view setOpenGLContext:opengl_context];
    [mirror_window setContentView: mirror_view];
    
    [mirror_window makeKeyAndOrderFront:Nil];
    
    [contexts addObject:opengl_context];
    int context_id = [contexts count] - 1;
    
    NSNumber *id_number = [NSNumber numberWithInt:context_id];
    display_refresh_rates.push_back(_measureDisplayRefreshRate(0));
    
    
    setCurrent(context_id);
    _initGlew();
        
    return context_id;
    
}


int OpenGLContextManager::newFullscreenContext(int pixelDepth, int screen_number){
    NSScreen *screen = _getScreen(screen_number);
    
        
    NSRect screen_rect = [screen frame];
//    NSLog(@"screen_rect: %g w, %g h, %g x, %g y", screen_rect.size.width,
//          screen_rect.size.height,
//          screen_rect.origin.x,
//          screen_rect.origin.y);
    
    // for some reason, some displays have random values here...
    screen_rect.origin.x = 0.0;
    screen_rect.origin.y = 0.0;
    
    fullscreen_window = [[NSWindow alloc] initWithContentRect:screen_rect 
                                                    styleMask:NSBorderlessWindowMask 
                                                      backing:NSBackingStoreBuffered 
                                                        defer:NO 
                                                       screen:screen];
    
    [fullscreen_window setLevel:NSMainMenuWindowLevel+1];
    
    [fullscreen_window setOpaque:YES];
    [fullscreen_window setHidesOnDeactivate:NO];
    
    NSOpenGLPixelFormatAttribute attrs[] =
    {
        NSOpenGLPFADoubleBuffer,
        0
    };
    
    
    NSOpenGLPixelFormat* pixel_format = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
    NSOpenGLContext *opengl_context = [[NSOpenGLContext alloc] initWithFormat:pixel_format shareContext:Nil];
    
    GLint swap_int = 1;
    [opengl_context setValues: &swap_int forParameter: NSOpenGLCPSwapInterval];
    
    
    NSRect view_rect = NSMakeRect(0.0, 0.0, screen_rect.size.width, screen_rect.size.height);
    
    fullscreen_view = [[NSOpenGLView alloc] initWithFrame:view_rect pixelFormat: pixel_format];
    [fullscreen_view setOpenGLContext:opengl_context];
    
    [fullscreen_window setContentView: fullscreen_view];
    
    [fullscreen_window makeKeyAndOrderFront:Nil];
    
    [contexts addObject:opengl_context];
    int context_id = [contexts count] - 1;
    
    setMainDisplayIndex(context_id);
    NSNumber *id_number = [NSNumber numberWithInt:context_id];
    
    display_refresh_rates.push_back(_measureDisplayRefreshRate(screen_number));
    
    setCurrent(context_id);
    _initGlew();
    
    glGenFencesAPPLE(1, &synchronization_fence);
    if(glIsFenceAPPLE(synchronization_fence)){
        has_fence = true;
    } else {
        has_fence = false;
    }
    
    return context_id;
}


//int OpenGLContextManager::newFullScreenContext(int pixelDepth, int index) {
//	
//    long swapParam = 1;
//    NSRect dRect;
//    NSOpenGLPixelFormat *fmt = nil;
//    NSOpenGLContext *stimOpenGLContext;
//    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
//    NSWindow *window = [[NSWindow alloc] init];
//    
//    //[window retain];
//    
//    [windows addObject:window];
//	
//	
//	int refresh_rate_hz = 60;
//	
//	shared_ptr<ComponentRegistry> reg = ComponentRegistry::getSharedRegistry();
//    shared_ptr<Variable> main_screen_info = reg->getVariable(MAIN_SCREEN_INFO_TAGNAME);
//
//    Datum val(main_screen_info->getValue());
//	if(val.hasKey(M_REFRESH_RATE_KEY)){
//		refresh_rate_hz = (int)(val.getElement(M_REFRESH_RATE_KEY));
//	}
//	
//	
//    //fullscreen = YES;
//	CGRect rect = CGDisplayBounds(display_ids[index]);
//    dRect = NSMakeRect(rect.origin.x, rect.origin.y, rect.size.width, rect.size.height);
//	
//    //[displays captureDisplayWithIndex:index];
//    CGDisplayCapture(display_ids[index]);
//	
//	if(!systemHasSecondMonitor()) {
//		CGDisplayHideCursor(display_ids[index]);
//	}
//	
//	
//	
//    CFDictionaryRef displayModeValues = 
//	CGDisplayBestModeForParametersAndRefreshRate(display_ids[index], 
//												 32, dRect.size.width, dRect.size.height, refresh_rate_hz, nil);
//    CGDisplaySwitchToMode(display_ids[index], displayModeValues);
//	
//	// check to see if desired refresh rate is the same as actual refresh rate
//	int actual_refresh_rate = getDisplayRefreshRate(index);
//	if(actual_refresh_rate != refresh_rate_hz) {
//		merror(M_DISPLAY_MESSAGE_DOMAIN, "Actual refresh rate (%d Hz) is different than refresh rate (%d) specified in setup variables", actual_refresh_rate, refresh_rate_hz);
//	}
//	
//	
//	
//	
//	/*	if (status == CGDisplayNoErr) {
//	 [displays dumpCurrentDisplayMode:index];
//	 }*/
//    
//	// DisplayParam display = [displays getDisplayParametersForIndex:index];
//	/* [window initWithContentRect:NSMakeRect(0, 0, display.widthPix, 
//	 display.heightPix)
//	 styleMask:NSBorderlessWindowMask 
//	 backing:NSBackingStoreBuffered defer:NO];*/
//	
//	[window initWithContentRect:NSMakeRect(0, 0, dRect.size.width, 
//										   dRect.size.height)
//					  styleMask:NSBorderlessWindowMask 
//						backing:NSBackingStoreBuffered defer:NO];
//    
//	// schedule a process to periodically announce the display's beam position
//	// Schedule beam announces
//	
//	// Remember our original display mode
//    originalDisplayMode = (NSDictionary *)CGDisplayCurrentMode(
//															   kCGDirectMainDisplay);
//    NSLog(@"originalDisplayMode = %@", originalDisplayMode);
//    
//	// Schedule beam announces
//	
//	// Does this system support beam position checks?
//	bool support_ok = false;
//	for(int i=0; i < N_BEAM_POSITION_SUPPORT_CHECKS; i++){
//		if(CGDisplayBeamPosition(display_ids[index])){
//			support_ok = true;
//			break;
//		}
//	}
//	
//	if(!support_ok){
//		
//		std::string beamMsg("The stimulus display does not support estimation of the "
//							"vertical refresh beam position.  This is commonly true on "
//							"LCD monitors and laptops (even with a CRT attached). Precise "
//							"measurement of frame timing will not be possible.");
//		
//		mwarning(M_DISPLAY_MESSAGE_DOMAIN,
//				 beamMsg.c_str());
//		
//		beamPosition->setValue(beamMsg);
//	} else {
//		
//		// leaks, but only once
//		struct beam_position_args *beam_args = new struct beam_position_args[1];
//		
//		beam_args->id = display_ids[index];
//		beam_args->display_height = 
//		[[originalDisplayMode objectForKey:(id)kCGDisplayHeight] intValue];
//		//fprintf(stderr, "(createNewContext) Display index is %d", targetDisplay); fflush(stderr);
//		
//		/*beamNode = GlobalScheduler->scheduleUS(FILELINE,
//		 0, 
//		 M_BEAM_POSITION_CHECK_INTERVAL_US, 
//		 M_REPEAT_INDEFINITELY, 
//		 &announce_beam_position, 
//		 (void *)beam_args,
//		 M_DEFAULT_PRIORITY,
//		 M_DEFAULT_WARN_SLOP_US,
//		 M_DEFAULT_FAIL_SLOP_US,
//		 M_MISSED_EXECUTION_CATCH_UP);*/
//	}
//	
//	/*	CGDirectDisplayID *index_to_send = new CGDirectDisplayID[1];
//	 *index_to_send = display_ids[index]; //[displays cgDirectDisplayIDForIndex:index];
//	 fprintf(stderr, "(newFullscreenContext) Display index is %d", index); fflush(stderr);
//	 beamNode = GlobalScheduler->scheduleUS(0, M_BEAM_POSITION_CHECK_INTERVAL, 
//	 M_REPEAT_INDEFINITELY, 
//	 &announce_beam_position, 
//	 (void *)index_to_send,
//	 M_DEFAULT_PRIORITY,
//	 M_DEFAULT_WARN_SLOP_MS,
//	 M_DEFAULT_FAIL_SLOP_MS,
//	 M_MISSED_EXECUTION_CATCH_UP);*/
//	
//	
//    // Now that we have a display_index, we can initialize the attributes
//	
//    NSOpenGLPixelFormatAttribute attribs[] =
//    {
//        NSOpenGLPFANoRecovery,
//        NSOpenGLPFAAccelerated,
//        NSOpenGLPFADoubleBuffer,
//        NSOpenGLPFAColorSize, (NSOpenGLPixelFormatAttribute) 24,
//        NSOpenGLPFAAlphaSize, (NSOpenGLPixelFormatAttribute) 8,
//        NSOpenGLPFADepthSize, (NSOpenGLPixelFormatAttribute) 0,
//        NSOpenGLPFAStencilSize, (NSOpenGLPixelFormatAttribute) 0,
//        NSOpenGLPFAAccumSize, (NSOpenGLPixelFormatAttribute) 0,
//		NSOpenGLPFABackingStore, (NSOpenGLPixelFormatAttribute) 1,
//        NSOpenGLPFAFullScreen,						// display to full screen
//        NSOpenGLPFAScreenMask, (NSOpenGLPixelFormatAttribute)
//		CGDisplayIDToOpenGLDisplayMask(display_ids[index]),
//        (NSOpenGLPixelFormatAttribute) 0				// nil terminator
//    };
//    
//    fmt = [[[NSOpenGLPixelFormat alloc] 
//			initWithAttributes:attribs] autorelease];
//    if (!fmt) {
//        NSLog(@"Cannot create NSOpenGLPixelFormat");
//		//TODO dependency problem
//		//mprintf("Failed to create fullscreen OpenGL pixel format");
//        [pool release];
//        return -1;
//    }
//    
//    stimOpenGLContext = [[NSOpenGLContext alloc] initWithFormat:fmt 
//												   shareContext:Nil];
//    //stimOpenGLContext = [[NSOpenGLContext alloc] 
//    //                         initWithFormat:fmt shareContext:global_context];
//    if (!stimOpenGLContext) {
//		//TODO dependency problem
//		//mprintf("OpenGL Context Manager: No (fullscreen) OpenGL context");
//        NSLog(@"No OpenGL context");
//        [pool release];
//        return -1;
//    }
//    
//    [stimOpenGLContext retain];
//    
//    //[glview setOpenGLContext:stimOpenGLContext];
//    [stimOpenGLContext setFullScreen];
//    [stimOpenGLContext makeCurrentContext];
//    [stimOpenGLContext setValues:(const GLint*)&swapParam 
//					forParameter:NSOpenGLCPSwapInterval];
//    
//    
//    glClearColor(0.5, 0.5, 0.5, 0.0);
//    glClear(GL_COLOR_BUFFER_BIT);
//    [stimOpenGLContext flushBuffer];
//    
//    [contexts addObject:stimOpenGLContext]; 
//    
//    // Close the mirror window
//    //[GlobalStimMirrorController hide];
//    
//	
//	
//    [pool release];
//    return [contexts count]-1;
//    
//}

void OpenGLContextManager::setMainDisplayIndex(const int index) { main_display_index = index; }

int OpenGLContextManager::getMainDisplayIndex() const {
	return main_display_index;
}

void OpenGLContextManager::setCurrent(int context_id) {
    if(context_id < 0 || context_id > [contexts count]) {
		mprintf("OpenGL Context Manager: no context to set current.");
		//NSLog(@"OpenGL Context Manager: no context to set current.");
        return;
    }
    [[contexts objectAtIndex:context_id] makeCurrentContext];     
}


void OpenGLContextManager::releaseDisplays() {
  
    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
    
    [contexts makeObjectsPerformSelector:@selector(clearDrawable)];
    
	CGReleaseAllDisplays();
    
    
    mirror_window_active = NO;
    if(mirror_window != Nil){
        [mirror_window orderOut:Nil];
        [mirror_window release];
        mirror_window = Nil;
    }
    
    
    fullscreen_window_active = NO;
    if(fullscreen_window != Nil){
        [fullscreen_window orderOut:Nil];
        [fullscreen_window release];
        fullscreen_window = Nil;
    }
    
	
    [contexts removeAllObjects];

    [pool release];

}


void OpenGLContextManager::flushCurrent() {
    [[NSOpenGLContext currentContext] flushBuffer];
}

void OpenGLContextManager::flush(int context_id, bool update) {
    if(context_id < 0){
        //TODO mprintf dependency problem
		NSLog(@"OpenGL Context Manager: no context to flush");
    }
    
	//glSetFenceAPPLE(synchronization_fence);
    if(update){
        [[contexts objectAtIndex:context_id] update];
    }
    [[contexts objectAtIndex:context_id] flushBuffer];
    
}

namespace mw {
	SINGLETON_INSTANCE_STATIC_DECLARATION(OpenGLContextManager)
}


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




#define ADD_ATTR(attr) \
do { \
attributeCount++; \
attributes = (NSOpenGLPixelFormatAttribute *)realloc(attributes, sizeof(*attributes) * attributeCount); \
attributes[attributeCount - 1] = attr; \
} while (0)

// may have to pass an autorelease pool into this object?
static NSOpenGLPixelFormat *_createPixelFormat(bool fullscreen, 
											   unsigned int colorBits,
											   unsigned int depthBits) {
    NSOpenGLPixelFormat *pixelFormat;
    NSOpenGLPixelFormatAttribute *attributes;
    unsigned int attributeCount = 0;
	
    attributes = (NSOpenGLPixelFormatAttribute *)malloc(sizeof(*attributes));
	
    if (fullscreen) {
        // Note that we want a fullscreen pixel format!
        ADD_ATTR(NSOpenGLPFAFullScreen);
    }
	
    ADD_ATTR(NSOpenGLPFAColorSize);
    ADD_ATTR((NSOpenGLPixelFormatAttribute)colorBits);
    ADD_ATTR(NSOpenGLPFAAlphaSize);
    ADD_ATTR((NSOpenGLPixelFormatAttribute)8);
	ADD_ATTR(NSOpenGLPFADepthSize);
    ADD_ATTR((NSOpenGLPixelFormatAttribute)depthBits);
    
    // We want double buffered and hardware accelerated
    ADD_ATTR(NSOpenGLPFADoubleBuffer);
    ADD_ATTR(NSOpenGLPFAAccelerated);
    
    // Note what display device we want to support
    ADD_ATTR(NSOpenGLPFAScreenMask);
    ADD_ATTR((NSOpenGLPixelFormatAttribute)CGDisplayIDToOpenGLDisplayMask(kCGDirectMainDisplay));
    
    // Terminate the list
    ADD_ATTR((NSOpenGLPixelFormatAttribute)0);
    
    pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attributes];
    free(attributes);
    
    return pixelFormat;
}


OpenGLContextManager::OpenGLContextManager() {
	
	mirror_window_active = NO;
    fullscreen_window_active = NO;
    
    mirror_window = Nil;
    fullscreen_window = Nil;
    
    contexts = [[NSMutableArray alloc] init];
    display_refresh_rates = [[NSMutableArray alloc] init];
    	
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
    
    if(index < 0 || [screens count] <= index){
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
    NSNumber *refresh_rate = [display_refresh_rates objectAtIndex:index];
    return [refresh_rate intValue];
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


//int OpenGLContextManager::newMirrorContext(int pixelDepth) {
//	NSOpenGLPixelFormat *pixelFormat;
//    NSOpenGLContext *context;
//	
//    NSRect rect;
//    NSDictionary *displayMode;
//    CGDisplayErr err;
//    CGLContextObj cglContext;
//    unsigned int colorBits;
//	bool fullscreen = false;
//
//    CGDisplayCount numDisplays;
//    CGDirectDisplayID targetDisplay;
//	
//	double width, height;
//    
//    err = CGGetActiveDisplayList(kMaxDisplays,display_ids,&numDisplays);
//    targetDisplay = display_ids[1];
//    
//	displayMode = currentDisplayMode; // currentDisplayMode is an object member
//    
//    
//    unsigned int depthBits = [[displayMode objectForKey:
//							   (id)kCGDisplayBitsPerPixel] intValue];
//    /*double width = [[displayMode objectForKey:(id)kCGDisplayWidth]
//	 intValue];
//	 double height = [[displayMode objectForKey:(id)kCGDisplayHeight]
//	 intValue];*/
//	
//	//    NSLog(@"modeIndex = %d", modeIndex);
//	//    NSLog(@"displayMode = %@", displayMode);  
//	
//	// We could also use one of the following functions to get a display mode 
//	// based on the size and color depth.  Note that the CG version has a bug
//	// currently in that it doesn't prefer unstretched modes on LCD displays.  
//	// The _gl_best_display_mode() does prefer unstretched modes.
//	
//	// displayMode = CGDisplayBestModeForParameters(kCGDirectMainDisplay, 
//	//                                          colorBits, width, height, NULL);
//	//    NSLog(@"CGDisplayBestModeForParameters -> %@", displayMode);
//	//    
//	//    //displayMode = _gl_best_display_mode(kCGDirectMainDisplay, width, 
//	//                                                      height, colorBits, NO);
//	//    if (!displayMode) {
//	//        mprintf("Unable to find a display mode matching these specifications");
//	//        //[self _error: @"Unable to find a display mode matching 
//	//                                                  these specifications."];
//	//        return;
//	//    }
//	//    NSLog(@"_gl_best_display_mode -> %@", displayMode);
//	
//    // Remember our original display mode
//    originalDisplayMode = (NSDictionary *)CGDisplayCurrentMode(
//															   kCGDirectMainDisplay);
//	//    NSLog(@"originalDisplayMode = %@", originalDisplayMode);
//    
//    // If we are NOT fullscreen, then we MUST have the same color depth
//    // as the current display mode
//	colorBits = [[originalDisplayMode objectForKey:
//				  (id)kCGDisplayBitsPerPixel] intValue];
//	
//	
//    // Figure out what display mode we want and switch to it.  
//    // Must do this before creating the GL context.  
//	// Otherwise, the GL context will say 'invalid drawable' if the old 
//    // display mode and new display mode 
//	// don't have the same bit depth.
//	
//	
//	
//	// Schedule beam announces
//	// Does this system support beam position checks?
//	bool support_ok = false;
//	for(int i=0; i < N_BEAM_POSITION_SUPPORT_CHECKS; i++){
//		if(CGDisplayBeamPosition(targetDisplay)){
//			support_ok = true;
//			break;
//		}
//	}
//	
//	if(!support_ok){
//		mwarning(M_DISPLAY_MESSAGE_DOMAIN,
//				 "The stimulus display does not support estimation of the vertical refresh beam position.  This is commonly true on LCD monitors and laptops (even with a CRT attached). Precise measurement of frame timing will not be possible.");
//	} else {
//		
//		/*		struct beam_position_args *beam_args = new struct beam_position_args[1];
//		 
//		 beam_args->id = targetDisplay;
//		 beam_args->display_height = 
//		 [[originalDisplayMode objectForKey:(id)kCGDisplayHeight] intValue];
//		 //fprintf(stderr, "(createNewContext) Display index is %d", targetDisplay); fflush(stderr);
//		 
//		 beamNode = GlobalScheduler->scheduleUS(0, M_BEAM_POSITION_CHECK_INTERVAL, 
//		 M_REPEAT_INDEFINITELY, 
//		 &announce_beam_position, 
//		 (void *)beam_args,
//		 M_DEFAULT_PRIORITY,
//		 M_DEFAULT_WARN_SLOP_MS,
//		 M_DEFAULT_FAIL_SLOP_MS,
//		 M_MISSED_EXECUTION_CATCH_UP);*/
//	}
//	
//    // Create a pixel format
//    pixelFormat = _createPixelFormat(fullscreen, colorBits, depthBits);
//    if (!pixelFormat) {
//        //TODO mprintf dependency problem
//		NSLog(@"Unable to find a pixel format matching these specs");
//        //[self _error: @"Unable to find a pixel format matching these 
//        //specifications."];
//        return NULL;
//    }
//    
//    // Create a GL context
//    context = [[NSOpenGLContext alloc] initWithFormat:pixelFormat 
//										 shareContext:nil];
//    [pixelFormat release];
//    if (!context) {
//        //TODO mprintf dependency problem
//		NSLog(@"Unable to create an OpenGL context");
//        //[self _error: @"Unable to create an OpenGL context."];
//        return NULL;
//    }
//    
//	
//	width = 960;
//	height = 600;
//	
//    shared_ptr<ComponentRegistry> reg = ComponentRegistry::getSharedRegistry();
//    shared_ptr<Variable> main_screen_info = reg->getVariable(MAIN_SCREEN_INFO_TAGNAME);
//  
//	if(main_screen_info != NULL){
//        Datum info = *main_screen_info;
//		
//		if(info.hasKey(M_DISPLAY_WIDTH_KEY) 
//		   && info.hasKey(M_DISPLAY_HEIGHT_KEY)
//		   && info.hasKey(M_MIRROR_WINDOW_BASE_HEIGHT_KEY)){
//			
//			double display_width, display_height, display_aspect;
//			display_width = info.getElement(M_DISPLAY_WIDTH_KEY);
//			display_height = info.getElement(M_DISPLAY_HEIGHT_KEY);
//			display_aspect = display_width / display_height;
//			height = (double)info.getElement(M_MIRROR_WINDOW_BASE_HEIGHT_KEY);
//			width = height * display_aspect;
//		}
//	}
//	
//	
//	// If we are not full screen, we'll need a window to 
//	// attach the GL context to
//	rect = NSMakeRect(0, 0, width, height);
//	mirrorWindow = [[NSWindow alloc] initWithContentRect:rect 
//											   styleMask:NSTitledWindowMask 
//												 backing:NSBackingStoreRetained 
//												   defer:NO];
//	[mirrorWindow orderFront: nil];
//	mirrorWindowActive = YES;
//	[windows addObject:mirrorWindow];
//	
//	// Attach the GL context to the content of the window
//	[context setView: [mirrorWindow contentView]];
//	//[windows addObject:window];
//    
//	
//    // Make the context we created be the current GL context
//    [context makeCurrentContext];
//    
//    // Get the underlying CGL context
//    cglContext = CGLGetCurrentContext();
//    
//    // Set the refresh sync on the context
//    {
//        const GLint param = 1;
//        
//        //CGLGetParameter(cglContext, kCGLCPSwapInterval, (GLint *)&param);
//		//        NSLog(@"original kCGLCPSwapInterval = %d", param);
//        
//        //param = [refreshLockButton state] ? 1 : 0;
//        
//        CGLSetParameter(cglContext, kCGLCPSwapInterval, &param);
//        
//        //CGLGetParameter(cglContext, kCGLCPSwapInterval, (GLint*)&param);
//        //NSLog(@"new kCGLCPSwapInterval = %d", param);
//    }
//    [contexts addObject: context];
//    return [contexts count] - 1;    
//}



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
    
    //[fullscreen_window setLevel:NSMainMenuWindowLevel+1];
    
    //[fullscreen_window setOpaque:YES];
    //[fullscreen_window setHidesOnDeactivate:YES];
    
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
    [display_refresh_rates addObject:[NSNumber numberWithDouble:_measureDisplayRefreshRate(0)]];
    return [contexts count] - 1;
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
        
    NSRect view_rect = NSMakeRect(0.0, 0.0, screen_rect.size.width, screen_rect.size.height);
    
    fullscreen_view = [[NSOpenGLView alloc] initWithFrame:view_rect pixelFormat: pixel_format];
    [fullscreen_view setOpenGLContext:opengl_context];
    
    [fullscreen_window setContentView: fullscreen_view];
    
    [fullscreen_window makeKeyAndOrderFront:Nil];
    
    [contexts addObject:opengl_context];
    [display_refresh_rates addObject:[NSNumber numberWithDouble:_measureDisplayRefreshRate(screen_number)]];
    return [contexts count] - 1;
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
    if(context_id < 0) {
		mprintf("OpenGL Context Manager: no context to set current.");
		//NSLog(@"OpenGL Context Manager: no context to set current.");
        return;
    }
    [[contexts objectAtIndex:context_id] makeCurrentContext];     
}


void OpenGLContextManager::releaseDisplays() {
    
    mirror_window_active = NO;
    if(mirror_window != Nil){
        [mirror_window orderOut:Nil];
    }
    
    
    fullscreen_window_active = NO;
    if(fullscreen_window != Nil){
        [fullscreen_window orderOut:Nil];
    }
    
	//NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
//	CGReleaseAllDisplays();
//	
//	if(mirrorWindowActive){
//		[mirrorWindow orderOut:Nil];
//		mirrorWindowActive = NO;
//	}
//	
//	[windows removeAllObjects];
//
//	
//    [pool release];
}


void OpenGLContextManager::flushCurrent() {
    [[NSOpenGLContext currentContext] flushBuffer];
}

void OpenGLContextManager::flush(int context_id) {
    if(context_id < 0){
        //TODO mprintf dependency problem
		NSLog(@"OpenGL Context Manager: no context to flush");
    }
    
	//glSetFenceAPPLE(synchronization_fence);
	[[contexts objectAtIndex:context_id] update];
    [[contexts objectAtIndex:context_id] flushBuffer];
    
}

namespace mw {
	SINGLETON_INSTANCE_STATIC_DECLARATION(OpenGLContextManager)
}


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

using namespace mw;





//******************************************************************
//******************************************************************
//                      Using Cocoa OpenGL
//******************************************************************
// *****************************************************************

#pragma mark  COCOA
#ifdef USE_COCOA_OPENGL

#include "GLWindow.h"
#import <OpenGL/OpenGL.h>
#import <OpenGL/gl.h>
#import <OpenGL/glu.h>
#import <OpenGL/glext.h>
#import <IOKit/graphics/IOGraphicsTypes.h>
#import "StateSystem.h"

#define kDefaultDisplay 1


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
			beamPosition->setValue(Data((double)beam_percent));
		}
	}
	
	return NULL;
}


@interface NSOpenGLContext(CGLContextAccess)
- (CGLContextObj)cglContext;
@end

@implementation NSOpenGLContext(CGLContextAccess)
- (CGLContextObj)cglContext; // this works?
{
    return _contextAuxiliary;
}
@end

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
	
	mirrorWindowActive = NO;
    contexts = [[NSMutableArray alloc] init];
    windows = [[NSMutableArray alloc] init];
    
	CGDisplayCount display_count;
	CGGetActiveDisplayList(kMaxDisplays, display_ids, &display_count);
	n_monitors_available = display_count;
	
	//displays = [[LLDisplayCalibration alloc] init];
    //n_monitors_available = [displays numDisplays];
    queryDisplayModes();
	
}

void OpenGLContextManager::initializeGlobalContext() {
    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
    NSOpenGLPixelFormatAttribute attribs[] =
    {
        NSOpenGLPFANoRecovery,
        NSOpenGLPFAAccelerated,
        NSOpenGLPFADoubleBuffer,
        NSOpenGLPFAColorSize, (NSOpenGLPixelFormatAttribute) 24,
        NSOpenGLPFAAlphaSize, (NSOpenGLPixelFormatAttribute) 8,
        NSOpenGLPFADepthSize, (NSOpenGLPixelFormatAttribute) 0,
        NSOpenGLPFAStencilSize, (NSOpenGLPixelFormatAttribute) 0,
        NSOpenGLPFAAccumSize, (NSOpenGLPixelFormatAttribute) 0,
        NSOpenGLPFAOffScreen,						
        NSOpenGLPFAScreenMask, 
		// TODO: something funky here... used to have "index" as argument...
        (NSOpenGLPixelFormatAttribute)
		CGDisplayIDToOpenGLDisplayMask(display_ids[0]),
        (NSOpenGLPixelFormatAttribute) 0				// nil terminator
    };
	
    NSOpenGLPixelFormat *fmt = [[[NSOpenGLPixelFormat alloc] 
								 initWithAttributes:attribs] autorelease];
    if (!fmt) {
        NSLog(@"Cannot create NSOpenGLPixelFormat");
        return;
    }
	
    global_context = [[NSOpenGLContext alloc] initWithFormat:fmt 
												shareContext:Nil];  
    [pool release];
}

bool OpenGLContextManager::systemHasSecondMonitor() {
    if(n_monitors_available > 1) {
        return true;
    }
	return false;
}

int OpenGLContextManager::getNMonitors() {
	return n_monitors_available;
}

int OpenGLContextManager::getDisplayWidth(const int index) {
    CFDictionaryRef modeInfo;
    int width = 0; // Assume LCD screen
	
    modeInfo = CGDisplayCurrentMode(display_ids[index]);
	
    if (modeInfo)
    {
        CFNumberRef value = (CFNumberRef) CFDictionaryGetValue(modeInfo, kCGDisplayWidth);
		
        if (value)
        {
            CFNumberGetValue(value, kCFNumberIntType, &width);
        }
    }
	
    return width;
}

int OpenGLContextManager::getDisplayHeight(const int index) {
    CFDictionaryRef modeInfo;
    int height = 0; // Assume LCD screen
	
    modeInfo = CGDisplayCurrentMode(display_ids[index]);
	
    if (modeInfo)
    {
        CFNumberRef value = (CFNumberRef) CFDictionaryGetValue(modeInfo, kCGDisplayHeight);
		
        if (value)
        {
            CFNumberGetValue(value, kCFNumberIntType, &height);
        }
    }
	
    return height;
}

int OpenGLContextManager::getDisplayRefreshRate(const int index)
{
    CFDictionaryRef modeInfo;
    int refreshRate = 60; // Assume LCD screen
	
    modeInfo = CGDisplayCurrentMode(display_ids[index]);
	
    if (modeInfo)
    {
        CFNumberRef value = (CFNumberRef) CFDictionaryGetValue(modeInfo, kCGDisplayRefreshRate);
		
        if (value)
        {
            CFNumberGetValue(value, kCFNumberIntType, &refreshRate);
            if (refreshRate == 0)
                refreshRate = 60;
        }
    }
	
    return refreshRate;
}


NSOpenGLContext *OpenGLContextManager::getGlobalContext() {
    return global_context;
}

void OpenGLContextManager::setGlobalContextCurrent() {
	//    [global_context setCurrent];
}

int OpenGLContextManager::newContext(int pixelDepth) {
	
	merror(M_DISPLAY_MESSAGE_DOMAIN, 
		   "Trying to use broken call in OpenGLContextManager");
	
    NSOpenGLPixelFormatAttribute attrs[] =
    {
        NSOpenGLPFADoubleBuffer,
        NSOpenGLPFADepthSize, (NSOpenGLPixelFormatAttribute) pixelDepth,
		(NSOpenGLPixelFormatAttribute) nil
    };
    
    NSOpenGLPixelFormat *pixformat = [[NSOpenGLPixelFormat alloc] 
									  initWithAttributes:attrs];
    
    NSOpenGLContext *newcontext = [[NSOpenGLContext alloc] 
								   initWithFormat:pixformat shareContext:nil];
    [contexts addObject:newcontext];
    return [contexts count]-1;
}

int OpenGLContextManager::newMirrorContext(int pixelDepth) {
	NSOpenGLPixelFormat *pixelFormat;
    NSOpenGLContext *context;
	//    NSWindow *window = nil;
    NSRect rect;
    NSDictionary *displayMode;
    CGDisplayErr err;
    CGLContextObj cglContext;
    unsigned int colorBits;
	bool fullscreen = false;
    //CGDirectDisplayID	displays[kMaxDisplays];
    CGDisplayCount numDisplays;
    CGDirectDisplayID targetDisplay;
	
	double width, height;
    
    err = CGGetActiveDisplayList(kMaxDisplays,display_ids,&numDisplays);
    targetDisplay = display_ids[1];
    
	displayMode = currentDisplayMode; // currentDisplayMode is an object member
    
    
    unsigned int depthBits = [[displayMode objectForKey:
							   (id)kCGDisplayBitsPerPixel] intValue];
    /*double width = [[displayMode objectForKey:(id)kCGDisplayWidth]
	 intValue];
	 double height = [[displayMode objectForKey:(id)kCGDisplayHeight]
	 intValue];*/
	
	//    NSLog(@"modeIndex = %d", modeIndex);
	//    NSLog(@"displayMode = %@", displayMode);  
	
	// We could also use one of the following functions to get a display mode 
	// based on the size and color depth.  Note that the CG version has a bug
	// currently in that it doesn't prefer unstretched modes on LCD displays.  
	// The _gl_best_display_mode() does prefer unstretched modes.
	
	// displayMode = CGDisplayBestModeForParameters(kCGDirectMainDisplay, 
	//                                          colorBits, width, height, NULL);
	//    NSLog(@"CGDisplayBestModeForParameters -> %@", displayMode);
	//    
	//    //displayMode = _gl_best_display_mode(kCGDirectMainDisplay, width, 
	//                                                      height, colorBits, NO);
	//    if (!displayMode) {
	//        mprintf("Unable to find a display mode matching these specifications");
	//        //[self _error: @"Unable to find a display mode matching 
	//                                                  these specifications."];
	//        return;
	//    }
	//    NSLog(@"_gl_best_display_mode -> %@", displayMode);
	
    // Remember our original display mode
    originalDisplayMode = (NSDictionary *)CGDisplayCurrentMode(
															   kCGDirectMainDisplay);
	//    NSLog(@"originalDisplayMode = %@", originalDisplayMode);
    
    // If we are NOT fullscreen, then we MUST have the same color depth
    // as the current display mode
	colorBits = [[originalDisplayMode objectForKey:
				  (id)kCGDisplayBitsPerPixel] intValue];
	
	
    // Figure out what display mode we want and switch to it.  
    // Must do this before creating the GL context.  
	// Otherwise, the GL context will say 'invalid drawable' if the old 
    // display mode and new display mode 
	// don't have the same bit depth.
	
	
	
	// Schedule beam announces
	// Does this system support beam position checks?
	bool support_ok = false;
	for(int i=0; i < N_BEAM_POSITION_SUPPORT_CHECKS; i++){
		if(CGDisplayBeamPosition(targetDisplay)){
			support_ok = true;
			break;
		}
	}
	
	if(!support_ok){
		mwarning(M_DISPLAY_MESSAGE_DOMAIN,
				 "The stimulus display does not support estimation of the vertical refresh beam position.  This is commonly true on LCD monitors and laptops (even with a CRT attached). Precise measurement of frame timing will not be possible.");
	} else {
		
		/*		struct beam_position_args *beam_args = new struct beam_position_args[1];
		 
		 beam_args->id = targetDisplay;
		 beam_args->display_height = 
		 [[originalDisplayMode objectForKey:(id)kCGDisplayHeight] intValue];
		 //fprintf(stderr, "(createNewContext) Display index is %d", targetDisplay); fflush(stderr);
		 
		 beamNode = GlobalScheduler->scheduleUS(0, M_BEAM_POSITION_CHECK_INTERVAL, 
		 M_REPEAT_INDEFINITELY, 
		 &announce_beam_position, 
		 (void *)beam_args,
		 M_DEFAULT_PRIORITY,
		 M_DEFAULT_WARN_SLOP_MS,
		 M_DEFAULT_FAIL_SLOP_MS,
		 M_MISSED_EXECUTION_CATCH_UP);*/
	}
	
    // Create a pixel format
    pixelFormat = _createPixelFormat(fullscreen, colorBits, depthBits);
    if (!pixelFormat) {
        //TODO mprintf dependency problem
		NSLog(@"Unable to find a pixel format matching these specs");
        //[self _error: @"Unable to find a pixel format matching these 
        //specifications."];
        return NULL;
    }
    
    // Create a GL context
    context = [[NSOpenGLContext alloc] initWithFormat:pixelFormat 
										 shareContext:nil];
    [pixelFormat release];
    if (!context) {
        //TODO mprintf dependency problem
		NSLog(@"Unable to create an OpenGL context");
        //[self _error: @"Unable to create an OpenGL context."];
        return NULL;
    }
    
	
	width = 960;
	height = 600;
	
	if(mainDisplayInfo != NULL){
		Data info = *mainDisplayInfo;
		
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
	
	
	// If we are not full screen, we'll need a window to 
	// attach the GL context to
	rect = NSMakeRect(0, 0, width, height);
	mirrorWindow = [[NSWindow alloc] initWithContentRect:rect 
											   styleMask:NSTitledWindowMask 
												 backing:NSBackingStoreRetained 
												   defer:NO];
	[mirrorWindow orderFront: nil];
	mirrorWindowActive = YES;
	[windows addObject:mirrorWindow];
	
	// Attach the GL context to the content of the window
	[context setView: [mirrorWindow contentView]];
	//[windows addObject:window];
    
	
    // Make the context we created be the current GL context
    [context makeCurrentContext];
    
    // Get the underlying CGL context
    cglContext = CGLGetCurrentContext();
    
    // Set the refresh sync on the context
    {
        long param;
        
        CGLGetParameter(cglContext, kCGLCPSwapInterval, (GLint *)&param);
		//        NSLog(@"original kCGLCPSwapInterval = %d", param);
        
        //param = [refreshLockButton state] ? 1 : 0;
        param = 1;
        //CGLSetParameter(cglContext, kCGLCPSwapInterval, &param);
        
        CGLGetParameter(cglContext, kCGLCPSwapInterval, (GLint*)&param);
        //NSLog(@"new kCGLCPSwapInterval = %d", param);
    }
    [contexts addObject: context];
    return [contexts count] - 1;    
}


int OpenGLContextManager::newFullScreenContext(int pixelDepth) {
	int index = 0;
    if(systemHasSecondMonitor()) {
		index = kDefaultDisplay;
    }
	
	return newFullScreenContext(pixelDepth, index);
}


int OpenGLContextManager::newFullScreenContext(int pixelDepth, int index) {
	
    long swapParam = 1;
    NSRect dRect;
    NSOpenGLPixelFormat *fmt = nil;
    NSOpenGLContext *stimOpenGLContext;
    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
    NSWindow *window = [[NSWindow alloc] init];
    
    //[window retain];
    
    [windows addObject:window];
	
	
	int refresh_rate_hz = 60;
	
	
	Data val(mainDisplayInfo->getValue());
	if(val.hasKey(M_REFRESH_RATE_KEY)){
		refresh_rate_hz = (int)(val.getElement(M_REFRESH_RATE_KEY));
	}
	
	
    //fullscreen = YES;
	CGRect rect = CGDisplayBounds(display_ids[index]);
    dRect = NSMakeRect(rect.origin.x, rect.origin.y, rect.size.width, rect.size.height);
	
    //[displays captureDisplayWithIndex:index];
    CGDisplayCapture(display_ids[index]);
	
	if(!systemHasSecondMonitor()) {
		CGDisplayHideCursor(display_ids[index]);
	}
	
	
	
    CFDictionaryRef displayModeValues = 
	CGDisplayBestModeForParametersAndRefreshRate(display_ids[index], 
												 24, dRect.size.width, dRect.size.height, refresh_rate_hz, nil);
    CGDisplaySwitchToMode(display_ids[index], displayModeValues);
	
	// check to see if desired refresh rate is the same as actual refresh rate
	int actual_refresh_rate = getDisplayRefreshRate(index);
	if(actual_refresh_rate != refresh_rate_hz) {
		merror(M_DISPLAY_MESSAGE_DOMAIN, "Actual refresh rate (%d Hz) is different than refresh rate (%d) specified in setup variables", actual_refresh_rate, refresh_rate_hz);
	}
	
	
	
	
	/*	if (status == CGDisplayNoErr) {
	 [displays dumpCurrentDisplayMode:index];
	 }*/
    
	// DisplayParam display = [displays getDisplayParametersForIndex:index];
	/* [window initWithContentRect:NSMakeRect(0, 0, display.widthPix, 
	 display.heightPix)
	 styleMask:NSBorderlessWindowMask 
	 backing:NSBackingStoreBuffered defer:NO];*/
	
	[window initWithContentRect:NSMakeRect(0, 0, dRect.size.width, 
										   dRect.size.height)
					  styleMask:NSBorderlessWindowMask 
						backing:NSBackingStoreBuffered defer:NO];
    
	// schedule a process to periodically announce the display's beam position
	// Schedule beam announces
	
	// Remember our original display mode
    originalDisplayMode = (NSDictionary *)CGDisplayCurrentMode(
															   kCGDirectMainDisplay);
    NSLog(@"originalDisplayMode = %@", originalDisplayMode);
    
	// Schedule beam announces
	
	// Does this system support beam position checks?
	bool support_ok = false;
	for(int i=0; i < N_BEAM_POSITION_SUPPORT_CHECKS; i++){
		if(CGDisplayBeamPosition(display_ids[index])){
			support_ok = true;
			break;
		}
	}
	
	if(!support_ok){
		
		std::string beamMsg("The stimulus display does not support estimation of the "
							"vertical refresh beam position.  This is commonly true on "
							"LCD monitors and laptops (even with a CRT attached). Precise "
							"measurement of frame timing will not be possible.");
		
		mwarning(M_DISPLAY_MESSAGE_DOMAIN,
				 beamMsg.c_str());
		
		beamPosition->setValue(beamMsg);
	} else {
		
		// leaks, but only once
		struct beam_position_args *beam_args = new struct beam_position_args[1];
		
		beam_args->id = display_ids[index];
		beam_args->display_height = 
		[[originalDisplayMode objectForKey:(id)kCGDisplayHeight] intValue];
		//fprintf(stderr, "(createNewContext) Display index is %d", targetDisplay); fflush(stderr);
		
		/*beamNode = GlobalScheduler->scheduleUS(FILELINE,
		 0, 
		 M_BEAM_POSITION_CHECK_INTERVAL_US, 
		 M_REPEAT_INDEFINITELY, 
		 &announce_beam_position, 
		 (void *)beam_args,
		 M_DEFAULT_PRIORITY,
		 M_DEFAULT_WARN_SLOP_US,
		 M_DEFAULT_FAIL_SLOP_US,
		 M_MISSED_EXECUTION_CATCH_UP);*/
	}
	
	/*	CGDirectDisplayID *index_to_send = new CGDirectDisplayID[1];
	 *index_to_send = display_ids[index]; //[displays cgDirectDisplayIDForIndex:index];
	 fprintf(stderr, "(newFullscreenContext) Display index is %d", index); fflush(stderr);
	 beamNode = GlobalScheduler->scheduleUS(0, M_BEAM_POSITION_CHECK_INTERVAL, 
	 M_REPEAT_INDEFINITELY, 
	 &announce_beam_position, 
	 (void *)index_to_send,
	 M_DEFAULT_PRIORITY,
	 M_DEFAULT_WARN_SLOP_MS,
	 M_DEFAULT_FAIL_SLOP_MS,
	 M_MISSED_EXECUTION_CATCH_UP);*/
	
	
    // Now that we have a display_index, we can initialize the attributes
	
    NSOpenGLPixelFormatAttribute attribs[] =
    {
        NSOpenGLPFANoRecovery,
        NSOpenGLPFAAccelerated,
        NSOpenGLPFADoubleBuffer,
        NSOpenGLPFAColorSize, (NSOpenGLPixelFormatAttribute) 24,
        NSOpenGLPFAAlphaSize, (NSOpenGLPixelFormatAttribute) 8,
        NSOpenGLPFADepthSize, (NSOpenGLPixelFormatAttribute) 0,
        NSOpenGLPFAStencilSize, (NSOpenGLPixelFormatAttribute) 0,
        NSOpenGLPFAAccumSize, (NSOpenGLPixelFormatAttribute) 0,
		// NSOpenGLPFABackingStore,(NSOpenGLPixelFormatAttribute) 1,
        NSOpenGLPFAFullScreen,						// display to full screen
        NSOpenGLPFAScreenMask, (NSOpenGLPixelFormatAttribute)
		CGDisplayIDToOpenGLDisplayMask(display_ids[index]),
        (NSOpenGLPixelFormatAttribute) 0				// nil terminator
    };
    
    fmt = [[[NSOpenGLPixelFormat alloc] 
			initWithAttributes:attribs] autorelease];
    if (!fmt) {
        NSLog(@"Cannot create NSOpenGLPixelFormat");
		//TODO dependency problem
		//mprintf("Failed to create fullscreen OpenGL pixel format");
        [pool release];
        return -1;
    }
    
    stimOpenGLContext = [[NSOpenGLContext alloc] initWithFormat:fmt 
												   shareContext:Nil];
    //stimOpenGLContext = [[NSOpenGLContext alloc] 
    //                         initWithFormat:fmt shareContext:global_context];
    if (!stimOpenGLContext) {
		//TODO dependency problem
		//mprintf("OpenGL Context Manager: No (fullscreen) OpenGL context");
        NSLog(@"No OpenGL context");
        [pool release];
        return -1;
    }
    
    [stimOpenGLContext retain];
    
    //[glview setOpenGLContext:stimOpenGLContext];
    [stimOpenGLContext setFullScreen];
    [stimOpenGLContext makeCurrentContext];
    [stimOpenGLContext setValues:(const GLint*)&swapParam 
					forParameter:NSOpenGLCPSwapInterval];
    
    
    glClearColor(0.5, 0.5, 0.5, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);
    [stimOpenGLContext flushBuffer];
    
    [contexts addObject:stimOpenGLContext]; 
    
    // Close the mirror window
    //[GlobalStimMirrorController hide];
    
	
	
    [pool release];
    return [contexts count]-1;
    
}

void OpenGLContextManager::setMainDisplayIndex(const int index) { main_display_index = index; }

int OpenGLContextManager::getMainDisplayIndex() const {
	return main_display_index;
}

void OpenGLContextManager::setCurrent(int context_id) {
    if(context_id < 0) {
		//TODO dependency problem
		//mprintf("OpenGL Context Manager: no context to set current.");
		NSLog(@"OpenGL Context Manager: no context to set current.");
        return;
    }
    [[contexts objectAtIndex:context_id] makeCurrentContext];     
	/* NSWindow *stimwind = [windows objectAtIndex:context_id];
	 if ([stimwind contentView]) {
	 [[stimwind contentView] lockFocus];
	 }*/
}

static int _compareModes(id arg1, id arg2, void *context) {
    NSDictionary *mode1 = (NSDictionary *)arg1;
    NSDictionary *mode2 = (NSDictionary *)arg2;
    int size1, size2;
    
    // Sort first on pixel count
    size1 = [[mode1 objectForKey: (NSString *)kCGDisplayWidth] intValue] *
	[[mode1 objectForKey: (NSString *)kCGDisplayHeight] intValue];
    size2 = [[mode2 objectForKey: (NSString *)kCGDisplayWidth] intValue] *
	[[mode2 objectForKey: (NSString *)kCGDisplayHeight] intValue];
    if (size1 != size2)
        return size1 - size2;
	
    // Then on bit depth
    return (int)[[mode1 objectForKey:
				  (NSString *)kCGDisplayBitsPerPixel] intValue] -
	(int)[[mode2 objectForKey:
		   (NSString *)kCGDisplayBitsPerPixel] intValue];
}

void OpenGLContextManager::queryDisplayModes() {
    unsigned int modeIndex, modeCount;
    NSArray *modes;
    NSDictionary *mode;
    NSString *description;
    unsigned int modeWidth, modeHeight, color, refresh, flags;
    bool preferedModeFound = false;
    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
    // Get the list of all available modes
    //modes = [(NSArray *)CGDisplayAvailableModes(kCGDirectMainDisplay) retain];
    modes = (NSArray *)CGDisplayAvailableModes(kCGDirectMainDisplay);
    
    // Filter out modes that we don't want
    displayModes = [[NSMutableArray alloc] init];
    modeCount = [modes count];
    for (modeIndex = 0; modeIndex < modeCount; modeIndex++) {
        mode = [modes objectAtIndex: modeIndex];
        color = [[mode objectForKey:(NSString *)kCGDisplayBitsPerPixel]
				 intValue];
        
        if (color < 16)
            continue;
        [displayModes addObject: mode];
    }
	
    // Sort the filtered modes
    [displayModes sortUsingFunction: _compareModes context: NULL];
	
    // Fill the popup with the resulting modes
    //[displayModePopUp removeAllItems];
    
    modeCount = [displayModes count];
    for (modeIndex = 0; modeIndex < modeCount; modeIndex++) {
        mode = [displayModes objectAtIndex: modeIndex];
        modeWidth = [[mode objectForKey:(NSString *)kCGDisplayWidth] intValue];
        modeHeight = [[mode objectForKey:(NSString *)kCGDisplayHeight] 
					  intValue];
        color = [[mode objectForKey:(NSString *)kCGDisplayBitsPerPixel]
				 intValue];
        refresh = [[mode objectForKey:(NSString *)kCGDisplayRefreshRate]
				   intValue];
        flags = [[mode objectForKey:(NSString *)kCGDisplayIOFlags] intValue];
        
        description = [NSString stringWithFormat:@"%dx%dx%d %dHz flags=0x%08x",
					   modeWidth, modeHeight, 
					   color, refresh, flags];
#define kPreferedScreenWidth 800
#define kPreferedScreenHeight 600
#define kPreferedRefreshRate 75
#define kPreferedColorDepth 24
        
        if(modeWidth == kPreferedScreenWidth && modeHeight == 
		   kPreferedScreenHeight && refresh == kPreferedRefreshRate) {
            currentDisplayMode = [displayModes objectAtIndex: modeIndex];
            preferedModeFound = true;
        }
        //[displayModePopUp addItemWithTitle: description];
    }
    
    if(!preferedModeFound){
        currentDisplayMode = [displayModes objectAtIndex:
							  [displayModes count] - 1];
    }
    //NSLog(@"displayModes = %@", displayModes);
    [pool release];
}

/*StimulusDisplay *OpenGLContextManager::createNewContext(int screen_mode) {
 }*/

void OpenGLContextManager::releaseDisplays() {
    
	NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
	CGReleaseAllDisplays();
	
	if(mirrorWindowActive){
		[mirrorWindow orderOut:Nil];
		mirrorWindowActive = NO;
	}
	
	[windows removeAllObjects];
	//	for(int i = 0; i < [windows count]; i++){
	//
	//		
	//		NSWindow *wind = (NSWindow *)[windows objectAtIndex:i];
	//			if(wind != NULL){
	//			//[wind hide];
	//			[wind release];
	//			[windows
	//		}
	//	}
	
    [pool release];
	
    //CGDisplayShowCursor(targetDisplay);
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
	
	// TODO: should be only for mirror
	[[contexts objectAtIndex:context_id] update];
    [[contexts objectAtIndex:context_id] flushBuffer];
    
    //[[[contexts objectAtIndex:context_id] view] setNeedsDisplay:YES];
    
	/* NSWindow *stimwind = [windows objectAtIndex:context_id];
	 if ([stimwind contentView]) {
	 [[stimwind contentView] unlockFocus];
	 }*/
}

#endif  // USE_COCOA_OPENGL









#pragma mark  SDL
/******************************************************************
 ******************************************************************
 *                      Using SDL OpenGL
 ******************************************************************
 *****************************************************************/

#ifdef  USE_SDL_OPENGL
#include "SDL/SDL.h"

void OpenGLContextManager::releaseDisplays() {
	// TODO: do something...
	//CGReleaseAllDisplays();
    //CGDisplayShowCursor(targetDisplay);
	
}


OpenGLContextManager::OpenGLContextManager() {
	
	initializeGlobalContext();
}

void OpenGLContextManager::queryDisplayModes() {	
    /* nothing to do  */
}


StimulusDisplay *OpenGLContextManager::createNewContext(int screen_mode) {
	if(screen_mode == M_FULLSCREEN_OPENGL_MODE) {
		return newFullScreenContext(32);
	} else {
		return newMirrorContext(32);
	}
}

StimulusDisplay *OpenGLContextManager::newContext(int pixelDepth) {   
    return newMirrorContext(pixelDepth);
}

StimulusDisplay *OpenGLContextManager::newMirrorContext(int pixelDepth) {
	Uint32 flags;
	int size;
	
	int width = 800;
	int height = 600;
	
	/* Initialize SDL */
	if ( SDL_Init(SDL_INIT_VIDEO) < 0 ) {
		fprintf(stderr, "Couldn't init SDL: %s\n", SDL_GetError());
		return NULL;
	}
	
	flags = SDL_OPENGL;
	//if ( fullscreenflag ) {
	//	flags |= SDL_FULLSCREEN;
	//}
	SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 1 );
	if ( SDL_SetVideoMode(width, height, 0, flags) == NULL ) {
		return NULL;
	}
	SDL_GL_GetAttribute( SDL_GL_STENCIL_SIZE, &size);
	//ReSizeGLScene(width, height);						// Set Up Our Perspective GL Screen
	glEnable(GL_TEXTURE_2D);							// Enable Texture Mapping
	glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
	glClearColor(0.00f, 0.00f, 0.00f, 0.0f);			// Black Background
	glClearDepth(1.0f);									// Depth Buffer Setup
	glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
	glDepthFunc(GL_LEQUAL);								// The Type Of Depth Testing To Do
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations
	
    return new StimulusDisplay(0);
}

StimulusDisplay *OpenGLContextManager::newFullScreenContext(int pixelDepth) {
	return newMirrorContext(pixelDepth);
}

bool OpenGLContextManager::systemHasSecondMonitor() {
	return false;	// SDL can't do multiple monitors
}

void OpenGLContextManager::initializeGlobalContext() {
	// do nothing
}

void OpenGLContextManager::setGlobalContextCurrent() {
	// nothing to do here.  Only one context!
}

void OpenGLContextManager::setCurrent(int context_id) {
	// nothing to do here.
}

void OpenGLContextManager::setContextToMirror(int context_id) {
	// nothing to do here.
}

void OpenGLContextManager::flush(int context_id) {
	flushCurrent(); // only one context
}

void OpenGLContextManager::flushCurrent() {
	glFlush();
	SDL_GL_SwapBuffers();
}        

#endif   // USE_SDL_OPENGL








/******************************************************************
 ******************************************************************
 *                      Using GLUT OpenGL
 ******************************************************************
 *****************************************************************/
#pragma mark  GLUT
#ifdef  USE_GLUT_OPENGL

void OpenGLContextManager::releaseDisplays() {
	// TODO: do something...
}

void glut_display_callback() {
	StimulusDisplay *stimdisplay = 
	GlobalCurrentExperiment->getStimulusDisplay();
	stimdisplay->updateDisplay();
}

OpenGLContextManager::OpenGLContextManager() {
	initializeGlobalContext();
}

void OpenGLContextManager::queryDisplayModes() {
    /* nothing to do  */
}

StimulusDisplay *OpenGLContextManager::createNewContext(int screen_mode) {
	if(screen_mode == M_FULLSCREEN_OPENGL_MODE) {
		return newFullScreenContext(32);
	} else {
		return newMirrorContext(32);
	}
}

StimulusDisplay *OpenGLContextManager::newContext(int pixelDepth) {
    return newMirrorContext(pixelDepth);
}

StimulusDisplay *OpenGLContextManager::newMirrorContext(int pixelDepth) {
	glutCreateWindow("Stimulus Display Mirror");
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowSize(800, 600);
	glutDisplayFunc(&glut_display_callback);
    return new StimulusDisplay(0);
}

StimulusDisplay *OpenGLContextManager::newFullScreenContext(int pixelDepth) {
	glutFullScreen();
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowSize(1024, 768); //TODO set this sensibly
	glutDisplayFunc(&glut_display_callback);
	return new StimulusDisplay(0);
}

bool OpenGLContextManager::systemHasSecondMonitor() {
	return false;	// glut can't do multiple monitors
}

void OpenGLContextManager::initializeGlobalContext() {
	// this actually appears to need to be done in main()
	//glutInit();
}

void OpenGLContextManager::setGlobalContextCurrent() {
	// nothing to do here.  Only one context!
}

void OpenGLContextManager::setCurrent(int context_id) {
	// nothing to do here.
}

void OpenGLContextManager::setContextToMirror(int context_id) {
	// nothing to do here.
}

void OpenGLContextManager::flush(int context_id) {
	flushCurrent(); // only one context
}

void OpenGLContextManager::flushCurrent() {
	//glutPostRedisplay();
	glFlush();
}

#endif   // USE_GLUT_OPENGL


namespace mw {
	// This should become a singleton thing eventually
	OpenGLContextManager *GlobalOpenGLContextManager;
}


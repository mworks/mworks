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
    
    display_sleep_block = kIOPMNullAssertionID;
    
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



int OpenGLContextManager::newMirrorContext(bool sync_to_vbl){
    
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

    NSOpenGLContext *mainContext = nil;
#if M_OPENGL_SHARED_STATE == 1
    if ([contexts count] > 0) {
        mainContext = [contexts objectAtIndex:0];
    }
#endif
    NSOpenGLContext *opengl_context = [[NSOpenGLContext alloc] initWithFormat:pixel_format shareContext:mainContext];
    
    if(sync_to_vbl){
        GLint swap_int = 1;
        [opengl_context setValues: &swap_int forParameter: NSOpenGLCPSwapInterval];
    }
    
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


int OpenGLContextManager::newFullscreenContext(int screen_number){
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
    
    if (kIOPMNullAssertionID == display_sleep_block) {
        if (kIOReturnSuccess != IOPMAssertionCreateWithName(kIOPMAssertionTypeNoDisplaySleep,
                                                            kIOPMAssertionLevelOn,
                                                            (CFStringRef)@"MWorks Prevent Display Sleep",
                                                            &display_sleep_block)) {
            mwarning(M_SERVER_MESSAGE_DOMAIN, "Cannot disable display sleep");
        }
    }
    
    return context_id;
}


void OpenGLContextManager::setMainDisplayIndex(const int index) { main_display_index = index; }

int OpenGLContextManager::getMainDisplayIndex() const {
	return main_display_index;
}

void OpenGLContextManager::setCurrent(int context_id) {
    if(context_id < 0 || context_id >= [contexts count]) {
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

    if (kIOPMNullAssertionID != display_sleep_block) {
        (void)IOPMAssertionRelease(display_sleep_block);  // Ignore the return code
        display_sleep_block = kIOPMNullAssertionID;
    }
	
    [contexts removeAllObjects];

    [pool release];


}


void OpenGLContextManager::flushCurrent() {
    [[NSOpenGLContext currentContext] flushBuffer];
}

void OpenGLContextManager::flush(int context_id, bool update) {
    if(context_id < 0 || context_id >= [contexts count]){
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


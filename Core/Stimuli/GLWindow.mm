/*

Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
                ("Apple") in consideration of your agreement to the following terms, and your
                use, installation, modification or redistribution of this Apple software
                constitutes acceptance of these terms.  If you do not agree with these terms,
                please do not use, install, modify or redistribute this Apple software.

                In consideration of your agreement to abide by the following terms, and subject
                to these terms, Apple grants you a personal, non-exclusive license, under Apple’s
                copyrights in this original Apple software (the "Apple Software"), to use,
                reproduce, modify and redistribute the Apple Software, with or without
                modifications, in source and/or binary forms; provided that if you redistribute
                the Apple Software in its entirety and without modifications, you must retain
                this notice and the following text and disclaimers in all such redistributions of
                the Apple Software.  Neither the name, trademarks, service marks or logos of
                Apple Computer, Inc. may be used to endorse or promote products derived from the
                Apple Software without specific prior written permission from Apple.  Except as
                expressly stated in this notice, no other rights or licenses, express or implied,
                are granted by Apple herein, including but not limited to any patent rights that
                may be infringed by your derivative works or by other works in which the Apple
                Software may be incorporated.

                The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
                WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
                WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
                PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
                COMBINATION WITH YOUR PRODUCTS.

                IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
                CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
                GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
                ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
                OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
                (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
                ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include <sys/time.h>

#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>

#include "DirectDisplay.h"

#import "GLWindow.h"

DirectDisplay displays;

static double get_time()
{
	struct timeval tod;
	
	gettimeofday(&tod, NULL);
	
	return tod.tv_sec + tod.tv_usec * 1.0E-6;
}

void DrawDisplay(const CGSize& size)
{
	static double t0 = get_time();

	double t = get_time() - t0;
	
	float t1 = fabs(sin(t));

	glClearColor(0,0,0.1*t1,0);
	glClear(GL_COLOR_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	GLint params[4];
	glGetIntegerv(GL_VIEWPORT, params);
	gluOrtho2D(0, params[2], params[3], 0);
	
	glMatrixMode(GL_MODELVIEW);

	glPushMatrix();

		glTranslatef(size.width*0.5f-64.0,0,0);
		glTranslatef((size.width*0.5f-64.0-(1-0.375))*sin(t*3),0,0);
		
		glBegin(GL_QUADS);

		glColor3f(fabs(cos(t)),0,fabs(sin(t)));
		glVertex2f(0.0, 0.375);
		
		glColor3f(fabs(cos(t)),fabs(sin(t)),0);
		glVertex2f(0.0, size.height);
		
		glColor3f(0,fabs(cos(t)),fabs(sin(t)));
		glVertex2f(128.0, size.height);
		
		glColor3f(fabs(sin(t)),0,fabs(cos(t)));
		glVertex2f(128.0, 0.375);

		glEnd();
		
		GLfloat frame[4] = { 1,1,1,1};
		glColor4fv(frame); 

		glBegin(GL_LINE_LOOP);

		glVertex2f(0.0, 0.375);
		
		glVertex2f(0.0, size.height);
		
		glVertex2f(128.0, size.height);
		
		glVertex2f(128.0, 0.375);
		
		glEnd();

	glPopMatrix();
}

@implementation GLWindow

// fullscreen initWithSize:

- (id) initWithSize: (NSSize) size
{
	// Capture display first:

	int display_index = 1; //don't use primary display

	displays.Init();

	displays.Capture(display_index);

	if (displays.SetDisplayMode(display_index, CGSizeMake(size.width, size.height), 32, 75))
		displays.DumpCurrentDisplayMode(display_index);
	
	[super initWithContentRect:NSMakeRect(0,0,size.width,size.height) styleMask:NSBorderlessWindowMask backing:NSBackingStoreBuffered defer:NO];

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
		NSOpenGLPFAFullScreen,
		NSOpenGLPFAScreenMask, (NSOpenGLPixelFormatAttribute) displays.GetOpenGLDisplayID(display_index),
		(NSOpenGLPixelFormatAttribute) 0
	};

	NSOpenGLPixelFormat* fmt = [[[NSOpenGLPixelFormat alloc] initWithAttributes: attribs] autorelease];

	if (!fmt)
	{
		NSLog(@"Cannot create NSOpenGLPixelFormat");
		return self;
	}
	
	ctx = [[NSOpenGLContext alloc] initWithFormat: fmt shareContext: nil];
	
	if (!ctx)
	{
		NSLog(@"No ctx");
		return self;
	}
	
	[ctx setFullScreen];
	[ctx makeCurrentContext];
	
	return self;
}

// windowed initWithFrame:

- (id) initWithFrame: (NSRect) bounds
{
	[super initWithContentRect:bounds styleMask:NSBorderlessWindowMask backing:NSBackingStoreBuffered defer:NO];

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
		NSOpenGLPFAWindow,
		(NSOpenGLPixelFormatAttribute) 0,
	};

	NSOpenGLPixelFormat* fmt = [[[NSOpenGLPixelFormat alloc] initWithAttributes: attribs] autorelease];

	if (!fmt)
	{
		NSLog(@"Cannot create NSOpenGLPixelFormat");
		return self;
	}
	
	NSRect view_bounds = NSMakeRect(0,0,bounds.size.width, bounds.size.height);
		
	[self setContentView: [[[NSOpenGLView alloc] initWithFrame:view_bounds pixelFormat:fmt] autorelease]];
	
	return self;
}


- (NSOpenGLContext*) context{
    return ctx;
}


- (void) update
{
	if ([self contentView])
		[[self contentView] lockFocus];

	long params[] = { vbl_state ? 1 : 0 };
	CGLSetParameter(CGLGetCurrentContext(),  kCGLCPSwapInterval, (const GLint*)params);

	NSRect bounds = [self frame]; 

        glClearColor(0.5, 0.5, 0.5, 0.0);
        glClear(GL_COLOR_BUFFER_BIT);

	//DrawDisplay(CGSizeMake(bounds.size.width, bounds.size.height));

	glFinish();
	[[NSOpenGLContext currentContext] flushBuffer];

	if ([self contentView])
		[[self contentView] unlockFocus];

}

@end

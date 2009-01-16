/* StimMirrorController */

#import <Cocoa/Cocoa.h>


@interface StimMirrorController : NSObject
{
    IBOutlet NSOpenGLView *glview;
}

//-(void)setCurrent;
-(void)setContext:(NSOpenGLContext *)newcontext;
-(NSWindow *)window;
-(NSRect)bounds;
- (void) setPixelFormat:(NSOpenGLPixelFormat *)fmt;
- (void) hide;
//- (void) update;
//- (void)flush;


@end



#import "StimMirrorController.h"
//#import "CommonUIDefinitions.h"


StimMirrorController *GlobalStimMirrorController;

@implementation StimMirrorController


- (id)init{

    self = [super init];
    //[glview openGLContext];
   // [glview setPixelFormat:[NSOpenGLView defaultPixelFormat]];

    return self;
}

- (void)dealloc{
    //[self clearGLContext];
	[glview dealloc];
	[super dealloc];
}


- (NSWindow *)window{

    return [glview window];
}


- (void) hide{
    
    [[self window] close];
}

- (void) setPixelFormat:(NSOpenGLPixelFormat *)fmt {
    [glview setPixelFormat:fmt];
}

- (NSRect)bounds {
    return [glview bounds];
}

- (void)setContext:(NSOpenGLContext *)newcontext{
    [glview setOpenGLContext:newcontext];
    [glview update];    
    //[newcontext setView:glview];
}


- (void)awakeFromNib{
    GlobalStimMirrorController = self;
}

@end

/**
 * main.m
 *
 * Description:  Launches the Application.
 *
 * History:
 * bkennedy on 7/05/07 - Created.
 *
 * Copyright MIT 2007 . All rights reserved.
 */

#import <Cocoa/Cocoa.h>

#import "MonkeyWorksCore/StandardServerCoreBuilder.h"
#import "MonkeyWorksCore/CoreBuilderForeman.h"

int main(int argc, char *argv[]) {
	using namespace mw;
	
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    NSApplication *myapp = [NSApplication sharedApplication];
    
    if(argc == 1) {
        // the program started by user clicking on application icon
        // the only argument is the name of the program.
    } else {
        // the user started from the client and we need to process arguments.        
    }
    
    // -----------------------------
    // Initialize the core
    // -----------------------------
    CoreBuilderForeman::constructCoreStandardOrder(new 
													StandardServerCoreBuilder());
	
	    
    // ----------------------------
    // Load Basic Cocoa Resources
    // ----------------------------
    [NSBundle loadNibNamed:@"MainMenu" owner:myapp];
	
	
    // ---------------------------------------
    // Set UI Running
    // ---------------------------------------    
    [myapp run];
    [pool release];
}

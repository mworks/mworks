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

#import "MWorksCore/StandardServerCoreBuilder.h"
#import "MWorksCore/CoreBuilderForeman.h"
#import "MWorksCore/Exceptions.h"

#import <Cocoa/Cocoa.h>

#import "MWSServer.h"


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
    
    NSError *error = Nil;
    
    try {
        CoreBuilderForeman::constructCoreStandardOrder(new 
													StandardServerCoreBuilder());
	} catch(ComponentFactoryConflictException& e){
        
        NSString *error_description = [NSString stringWithCString:e.getMessage().c_str() encoding:NSASCIIStringEncoding];
        NSString *recovery_suggestion = @"You must review your plugins to ensure that multiple plugins aren't trying to register functionality under the same XML signatures";
        NSMutableDictionary *error_info = [[NSMutableDictionary alloc] init];
        [error_info setObject: error_description  forKey: NSLocalizedDescriptionKey];
        [error_info setObject: recovery_suggestion forKey: NSLocalizedRecoverySuggestionErrorKey];
        
        error = [NSError errorWithDomain:@"PluginLoader" 
                         code: 100 
                        userInfo: error_info];
    }
	    
    // ----------------------------
    // Load Basic Cocoa Resources
    // ----------------------------
    [NSBundle loadNibNamed:@"MainMenu" owner:myapp];
	
	if(error != Nil){
        [(MWSServer *)[myapp delegate] setError:error];
    }
    
    // ---------------------------------------
    // Set UI Running
    // ---------------------------------------    
    [myapp run];
    [pool release];
}

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


int main(int argc, const char *argv[]) {
	mw::CoreBuilderForeman::constructCoreStandardOrder(new mw::StandardServerCoreBuilder());	
	
	return NSApplicationMain(argc, argv); 
}
//
//    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
//    NSApplication *marionette_app = [NSApplication sharedApplication];
//    
//    // -----------------------------
//    // Initialize the core
//    // -----------------------------
//	
//	
//    // ----------------------------
//    // Load Basic Cocoa Resources
//    // ----------------------------
//    [NSBundle loadNibNamed:@"Marionette" owner:marionette_app];
//	
//	
//    // ---------------------------------------
//    // Set UI Running
//    // ---------------------------------------    
//    [marionette_app run];
//    [pool release];
//}


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
#import <Cocoa/Cocoa.h>

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


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
#import <IOKit/IOKitLib.h>

#import "MWorksCore/StandardClientCoreBuilder.h"
#import "MWorksCore/CoreBuilderForeman.h"

int main(int argc, const char *argv[]) {
    // Ensure that the display is awake
    io_registry_entry_t entry = IORegistryEntryFromPath(kIOMainPortDefault,
                                                        "IOService:/IOResources/IODisplayWrangler");
    if (entry) {
        (void)IORegistryEntrySetCFProperty(entry, CFSTR("IORequestIdle"), kCFBooleanFalse);
        (void)IOObjectRelease(entry);
    }
    
    mw::StandardClientCoreBuilder coreBuilder;
    mw::CoreBuilderForeman::constructCoreStandardOrder(&coreBuilder);
    
    return NSApplicationMain(argc, argv);
}

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

#import "MWorksCore/StandardServerCoreBuilder.h"
#import "MWorksCore/CoreBuilderForeman.h"

int main(int argc, const char *argv[]) {
    // Ensure that the display is awake
    io_registry_entry_t entry = IORegistryEntryFromPath(kIOMasterPortDefault,
                                                        "IOService:/IOResources/IODisplayWrangler");
    if (entry) {
        (void)IORegistryEntrySetCFProperty(entry, CFSTR("IORequestIdle"), kCFBooleanFalse);
        (void)IOObjectRelease(entry);
    }
    
    mw::StandardServerCoreBuilder coreBuilder;
    mw::CoreBuilderForeman::constructCoreStandardOrder(&coreBuilder);
    
    return NSApplicationMain(argc, argv);
}

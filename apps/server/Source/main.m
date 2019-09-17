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

@import Cocoa;

@import MWorksSwift;


int main(int argc, char *argv[]) {
    @autoreleasepool {
        NSError *error = nil;
        if (![MWKCore constructCoreWithType:MWKCoreTypeServer error:&error]) {
            [NSApplication.sharedApplication presentError:error];
            return EXIT_FAILURE;
        }
    }
    return NSApplicationMain(argc, (const char **)argv);
}

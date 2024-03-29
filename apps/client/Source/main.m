//
//  main.m
//  New Client
//
//  Created by David Cox on 3/10/08.
//  Copyright Harvard University 2008. All rights reserved.
//

@import Cocoa;

@import MWorksSwift;


int main(int argc, char *argv[]) {
    @autoreleasepool {
        NSError *error = nil;
        if (![MWKCore constructCoreWithType:MWKCoreTypeClient error:&error]) {
            [NSApplication.sharedApplication presentError:error];
            return EXIT_FAILURE;
        }
    }
    return NSApplicationMain(argc, (const char **)argv);
}

//
//  main.m
//  New Client
//
//  Created by David Cox on 3/10/08.
//  Copyright Harvard University 2008. All rights reserved.
//

#import <Cocoa/Cocoa.h>

#import <MWorksSwift/MWorksSwift.h>


int main(int argc, char *argv[]) {
    @autoreleasepool {
        NSError *error = nil;
        if (![MWKClient constructCore:&error]) {
            [NSApplication.sharedApplication presentError:error];
            return EXIT_FAILURE;
        }
    }
    return NSApplicationMain(argc, (const char **)argv);
}

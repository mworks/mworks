//
//  AppDelegate.m
//  MWServer-iOS
//
//  Created by Christopher Stawarz on 2/17/17.
//
//

#import "AppDelegate.h"


@implementation AppDelegate {
    id<NSObject> ioActivity;
}


- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    // Ensure that Foundation is set up for multiple threads
    NSAssert(NSThread.isMultiThreaded, @"Foundation multithreading is not enabled");
    
    // Prevent system sleep
    application.idleTimerDisabled = YES;
    
    // Take a power assertion to prevent the OS from throttling long-running event listener and I/O threads.
    // (Not sure if this actually helps on iOS, but it probably can't hurt.)
    ioActivity = [NSProcessInfo.processInfo beginActivityWithOptions:(NSActivityBackground | NSActivityIdleSystemSleepDisabled)
                                                              reason:@"Prevent I/O throttling"];
    
    return YES;
}


- (void)applicationWillTerminate:(UIApplication *)application {
    // Release power assertion
    [NSProcessInfo.processInfo endActivity:ioActivity];
    
    // Re-enable system sleep
    application.idleTimerDisabled = NO;
}


@end

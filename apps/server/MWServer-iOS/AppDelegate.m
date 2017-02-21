//
//  AppDelegate.m
//  MWServer-iOS
//
//  Created by Christopher Stawarz on 2/17/17.
//
//

#import "AppDelegate.h"

#include <MWorksCore/CoreBuilderForeman.h>
#include <MWorksCore/Server.h>
#include <MWorksCore/StandardServerCoreBuilder.h>


@implementation AppDelegate {
    boost::shared_ptr<mw::Server> core;
}


static UIAlertController * createInitializationFailureAlert(NSString *message) {
    return [UIAlertController alertControllerWithTitle:@"Server initialization failed"
                                               message:message
                                        preferredStyle:UIAlertControllerStyleAlert];
}


- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    try {
        
        mw::StandardServerCoreBuilder coreBuilder;
        mw::CoreBuilderForeman::constructCoreStandardOrder(&coreBuilder);
        
        core = boost::make_shared<mw::Server>();
        mw::Server::registerInstance(core);
        
        core->startServer();
        
    } catch (const std::exception &e) {
        self.alert = createInitializationFailureAlert(@(e.what()));
    } catch (...) {
        self.alert = createInitializationFailureAlert(@"An unknown error occurred");
    }
    
    return YES;
}


- (void)applicationWillResignActive:(UIApplication *)application {
    // Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
    // Use this method to pause ongoing tasks, disable timers, and invalidate graphics rendering callbacks. Games should use this method to pause the game.
}


- (void)applicationDidEnterBackground:(UIApplication *)application {
    // Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later.
    // If your application supports background execution, this method is called instead of applicationWillTerminate: when the user quits.
}


- (void)applicationWillEnterForeground:(UIApplication *)application {
    // Called as part of the transition from the background to the active state; here you can undo many of the changes made on entering the background.
}


- (void)applicationDidBecomeActive:(UIApplication *)application {
    // Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
}


- (void)applicationWillTerminate:(UIApplication *)application {
    if (core) {
        try {
            core->stopServer();
        } catch (const std::exception &e) {
            NSLog(@"Exception in stopServer: %s", e.what());
        } catch (...) {
            NSLog(@"Unknown exception in stopServer");
        }
    }
}


@end





























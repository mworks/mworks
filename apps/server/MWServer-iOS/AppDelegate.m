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

#define SERVER_NAME_PREFERENCE @"server_name_preference"
#define LISTENING_PORT_PREFERENCE @"listening_port_preference"
#define ALLOW_ALT_FAILOVER_PREFERENCE @"allow_alt_failover_preference"
#define DISPLAY_WIDTH_PREFERENCE @"display_width_preference"
#define DISPLAY_HEIGHT_PREFERENCE @"display_height_preference"
#define DISPLAY_DISTANCE_PREFERENCE @"display_distance_preference"
#define ANNOUNCE_INDIVIDUAL_STIMULI_PREFERENCE @"announce_individual_stimuli_preference"
#define WARN_ON_SKIPPED_REFRESH_PREFERENCE @"warn_on_skipped_refresh_preference"


@implementation AppDelegate {
    boost::shared_ptr<mw::Server> core;
}


static void registerDefaultSettings(NSUserDefaults *userDefaults) {
    NSDictionary *defaultSettings =
    @{
      LISTENING_PORT_PREFERENCE: @(19989),
      ALLOW_ALT_FAILOVER_PREFERENCE: @(YES),
      ANNOUNCE_INDIVIDUAL_STIMULI_PREFERENCE: @(YES),
      WARN_ON_SKIPPED_REFRESH_PREFERENCE: @(YES)
      };
    [userDefaults registerDefaults:defaultSettings];
}


static void initializeSetupVariables(NSUserDefaults *userDefaults,
                                     MWSSetupVariablesController *setupVariablesController)
{
    NSString *serverName = [userDefaults stringForKey:SERVER_NAME_PREFERENCE];
    if (!serverName || serverName.length == 0) {
        serverName = UIDevice.currentDevice.name;
    }
    setupVariablesController.serverName = serverName;
    
    setupVariablesController.displayToUse = @(1);
    {
        double displayWidth = [userDefaults doubleForKey:DISPLAY_WIDTH_PREFERENCE];
        if (displayWidth != 0.0) {
            setupVariablesController.displayWidth = @(displayWidth);
        }
    }
    {
        double displayHeight = [userDefaults doubleForKey:DISPLAY_HEIGHT_PREFERENCE];
        if (displayHeight != 0.0) {
            setupVariablesController.displayHeight = @(displayHeight);
        }
    }
    {
        double displayDistance = [userDefaults doubleForKey:DISPLAY_DISTANCE_PREFERENCE];
        if (displayDistance != 0.0) {
            setupVariablesController.displayDistance = @(displayDistance);
        }
    }
    setupVariablesController.displayRefreshRateHz = @(60.0);
    setupVariablesController.alwaysDisplayMirrorWindow = NO;
    setupVariablesController.mirrorWindowBaseHeight = @(0);
    setupVariablesController.announceIndividualStimuli = [userDefaults boolForKey:ANNOUNCE_INDIVIDUAL_STIMULI_PREFERENCE];
    setupVariablesController.warnOnSkippedRefresh = [userDefaults boolForKey:WARN_ON_SKIPPED_REFRESH_PREFERENCE];
    
    setupVariablesController.allowAltFailover = [userDefaults boolForKey:ALLOW_ALT_FAILOVER_PREFERENCE];
}


static UIAlertController * createInitializationFailureAlert(NSString *message) {
    return [UIAlertController alertControllerWithTitle:@"Server initialization failed"
                                               message:message
                                        preferredStyle:UIAlertControllerStyleAlert];
}


- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    // Prevent system sleep
    application.idleTimerDisabled = YES;
    
    try {
        
        mw::StandardServerCoreBuilder coreBuilder;
        mw::CoreBuilderForeman::constructCoreStandardOrder(&coreBuilder);
        
        core = boost::make_shared<mw::Server>();
        mw::Server::registerInstance(core);
        
        NSUserDefaults *userDefaults = NSUserDefaults.standardUserDefaults;
        registerDefaultSettings(userDefaults);
        _setupVariablesController = [[MWSSetupVariablesController alloc] init];
        initializeSetupVariables(userDefaults, self.setupVariablesController);
        
#if TARGET_OS_SIMULATOR
        _listeningAddress = @"127.0.0.1";
#else
        _listeningAddress = NSProcessInfo.processInfo.hostName;
#endif
        core->setHostname(self.listeningAddress.UTF8String);
        
        _listeningPort = @([userDefaults integerForKey:LISTENING_PORT_PREFERENCE]);
        core->setListenPort(self.listeningPort.intValue);
        
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
    
    // Re-enable system sleep
    application.idleTimerDisabled = NO;
}


@end





























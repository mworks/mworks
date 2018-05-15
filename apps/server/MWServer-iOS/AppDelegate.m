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
#include <MWorksCore/ZeroMQUtilities.hpp>

#define SERVER_NAME_PREFERENCE @"server_name_preference"
#define LISTENING_PORT_PREFERENCE @"listening_port_preference"
#define ALLOW_ALT_FAILOVER_PREFERENCE @"allow_alt_failover_preference"
#define STOP_ON_ERROR_PREFERENCE @"stop_on_error_preference"
#define WARN_ON_SKIPPED_REFRESH_PREFERENCE @"warn_on_skipped_refresh_preference"
#define DISPLAY_WIDTH_PREFERENCE @"display_width_preference"
#define DISPLAY_HEIGHT_PREFERENCE @"display_height_preference"
#define DISPLAY_DISTANCE_PREFERENCE @"display_distance_preference"
#define ANNOUNCE_INDIVIDUAL_STIMULI_PREFERENCE @"announce_individual_stimuli_preference"
#define RENDER_AT_FULL_RESOLUTION_PREFERENCE @"render_at_full_resolution_preference"
#define USE_COLOR_MANAGEMENT_PREFERENCE @"use_color_management_preference"


@implementation AppDelegate {
    boost::shared_ptr<mw::Server> core;
}


static void registerDefaultSettings(NSUserDefaults *userDefaults) {
    NSDictionary *defaultSettings =
    @{
      LISTENING_PORT_PREFERENCE: @(19989),
      ALLOW_ALT_FAILOVER_PREFERENCE: @(YES),
      STOP_ON_ERROR_PREFERENCE: @(NO),
      WARN_ON_SKIPPED_REFRESH_PREFERENCE: @(YES),
      ANNOUNCE_INDIVIDUAL_STIMULI_PREFERENCE: @(YES),
      RENDER_AT_FULL_RESOLUTION_PREFERENCE: @(YES),
      USE_COLOR_MANAGEMENT_PREFERENCE: @(YES)
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
    setupVariablesController.renderAtFullResolution = [userDefaults boolForKey:RENDER_AT_FULL_RESOLUTION_PREFERENCE];
    setupVariablesController.useColorManagement = [userDefaults boolForKey:USE_COLOR_MANAGEMENT_PREFERENCE];
    
    setupVariablesController.warnOnSkippedRefresh = [userDefaults boolForKey:WARN_ON_SKIPPED_REFRESH_PREFERENCE];
    setupVariablesController.stopOnError = [userDefaults boolForKey:STOP_ON_ERROR_PREFERENCE];
    setupVariablesController.allowAltFailover = [userDefaults boolForKey:ALLOW_ALT_FAILOVER_PREFERENCE];
}


static UIAlertController * createInitializationFailureAlert(NSString *message) {
    return [UIAlertController alertControllerWithTitle:@"Server initialization failed"
                                               message:message
                                        preferredStyle:UIAlertControllerStyleAlert];
}


- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    // Ensure that Foundation is set up for multiple threads
    NSAssert(NSThread.isMultiThreaded, @"Foundation multithreading is not enabled");
    
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
        _listeningAddress = @"localhost";
        core->setHostname(self.listeningAddress.UTF8String);
#else
        std::string hostname;
        if (mw::zeromq::getHostname(hostname)) {
            _listeningAddress = @(hostname.c_str());
        } else {
            // This is less reliable than zeromq::getHostname, so use it only as a fallback
            _listeningAddress = NSProcessInfo.processInfo.hostName;
        }
#endif
        
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





























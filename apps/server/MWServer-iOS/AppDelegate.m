//
//  AppDelegate.m
//  MWServer-iOS
//
//  Created by Christopher Stawarz on 2/17/17.
//
//

#import "AppDelegate.h"
#import "ViewController.h"

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
#define USE_COLOR_MANAGEMENT_PREFERENCE @"use_color_management_preference"
#define DID_INSTALL_EXAMPLE_EXPERIMENTS_PREFERENCE @"did_install_example_experiments_preference"

#define EVENT_CALLBACK_KEY "<MWServer-iOS/AppDelegate>"
#define PERSISTENT_EVENT_CALLBACK_KEY "<MWServer-iOS/AppDelegate-Persistent>"


static void registerDefaultSettings(NSUserDefaults *userDefaults) {
    NSDictionary *defaultSettings =
    @{
      LISTENING_PORT_PREFERENCE: @(19989),
      ALLOW_ALT_FAILOVER_PREFERENCE: @(YES),
      STOP_ON_ERROR_PREFERENCE: @(NO),
      WARN_ON_SKIPPED_REFRESH_PREFERENCE: @(YES),
      DISPLAY_WIDTH_PREFERENCE: @(373.33),
      DISPLAY_HEIGHT_PREFERENCE: @(280),
      DISPLAY_DISTANCE_PREFERENCE: @(450),
      ANNOUNCE_INDIVIDUAL_STIMULI_PREFERENCE: @(YES),
      USE_COLOR_MANAGEMENT_PREFERENCE: @(YES),
      DID_INSTALL_EXAMPLE_EXPERIMENTS_PREFERENCE: @(NO)
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
    setupVariablesController.displayWidth = @([userDefaults doubleForKey:DISPLAY_WIDTH_PREFERENCE]);
    setupVariablesController.displayHeight = @([userDefaults doubleForKey:DISPLAY_HEIGHT_PREFERENCE]);
    setupVariablesController.displayDistance = @([userDefaults doubleForKey:DISPLAY_DISTANCE_PREFERENCE]);
    setupVariablesController.displayRefreshRateHz = @(60.0);
    setupVariablesController.alwaysDisplayMirrorWindow = NO;
    setupVariablesController.mirrorWindowBaseHeight = @(0);
    setupVariablesController.announceIndividualStimuli = [userDefaults boolForKey:ANNOUNCE_INDIVIDUAL_STIMULI_PREFERENCE];
    setupVariablesController.useColorManagement = [userDefaults boolForKey:USE_COLOR_MANAGEMENT_PREFERENCE];
    
    setupVariablesController.warnOnSkippedRefresh = [userDefaults boolForKey:WARN_ON_SKIPPED_REFRESH_PREFERENCE];
    setupVariablesController.stopOnError = [userDefaults boolForKey:STOP_ON_ERROR_PREFERENCE];
    setupVariablesController.allowAltFailover = [userDefaults boolForKey:ALLOW_ALT_FAILOVER_PREFERENCE];
}


static void installExampleExperiments(NSUserDefaults *userDefaults) {
    if (![userDefaults boolForKey:DID_INSTALL_EXAMPLE_EXPERIMENTS_PREFERENCE]) {
        NSFileManager *fileManager = [NSFileManager defaultManager];
        NSURL *examplesURL = [NSBundle.mainBundle URLForResource:@"Examples" withExtension:nil];
        NSURL *documentsURL = [fileManager URLForDirectory:NSDocumentDirectory
                                                  inDomain:NSUserDomainMask
                                         appropriateForURL:nil
                                                    create:YES
                                                     error:nil];
        [fileManager copyItemAtURL:examplesURL
                             toURL:[documentsURL URLByAppendingPathComponent:@"Examples" isDirectory:YES]
                             error:nil];
        [userDefaults setBool:YES forKey:DID_INSTALL_EXAMPLE_EXPERIMENTS_PREFERENCE];
    }
}


static UIAlertController * createInitializationFailureAlert(NSString *message) {
    return [UIAlertController alertControllerWithTitle:@"Server initialization failed"
                                               message:message
                                        preferredStyle:UIAlertControllerStyleAlert];
}


@implementation AppDelegate {
    boost::shared_ptr<mw::Server> core;
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
    
    try {
        
        mw::StandardServerCoreBuilder coreBuilder;
        mw::CoreBuilderForeman::constructCoreStandardOrder(&coreBuilder);
        
        core = boost::make_shared<mw::Server>();
        mw::Server::registerInstance(core);
        
        NSUserDefaults *userDefaults = NSUserDefaults.standardUserDefaults;
        registerDefaultSettings(userDefaults);
        _setupVariablesController = [[MWSSetupVariablesController alloc] init];
        initializeSetupVariables(userDefaults, self.setupVariablesController);
        installExampleExperiments(userDefaults);
        
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
        
        auto systemEventCallback = [](const boost::shared_ptr<mw::Event> &event) {
            auto &data = event->getData();
            if (data.getElement(M_SYSTEM_PAYLOAD_TYPE).getInteger() == mw::M_EXPERIMENT_STATE) {
                auto loaded = data.getElement(M_SYSTEM_PAYLOAD).getElement(M_LOADED).getBool();
                dispatch_async(dispatch_get_main_queue(), ^{
                    ViewController *viewController = (ViewController *)(APP_DELEGATE.window.rootViewController);
                    viewController.chooseExperiment.enabled = !loaded;
                });
            }
        };
        core->registerCallback(mw::RESERVED_SYSTEM_EVENT_CODE, systemEventCallback, PERSISTENT_EVENT_CALLBACK_KEY);
        
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
            core->unregisterCallbacks(PERSISTENT_EVENT_CALLBACK_KEY);
            core->stopServer();
        } catch (const std::exception &e) {
            NSLog(@"Exception in %s: %s", __PRETTY_FUNCTION__, e.what());
        } catch (...) {
            NSLog(@"Unknown exception in %s", __PRETTY_FUNCTION__);
        }
    }
    
    // Release power assertion
    [NSProcessInfo.processInfo endActivity:ioActivity];
    
    // Re-enable system sleep
    application.idleTimerDisabled = NO;
}


- (void)openExperiment:(NSString *)path completionHandler:(void (^)(BOOL success))completionHandler {
    try {
        boost::weak_ptr<mw::Server> weakCore(core);
        auto systemEventCallback = [weakCore, completionHandler](const boost::shared_ptr<mw::Event> &event) {
            auto &data = event->getData();
            if (data.getElement(M_SYSTEM_PAYLOAD_TYPE).getInteger() == mw::M_EXPERIMENT_STATE) {
                if (auto core = weakCore.lock()) {
                    core->unregisterCallbacks(EVENT_CALLBACK_KEY);
                }
                auto success = data.getElement(M_SYSTEM_PAYLOAD).getElement(M_LOADED).getBool();
                dispatch_async(dispatch_get_main_queue(), ^{
                    completionHandler(success);
                    if (success) {
                        [APP_DELEGATE openExperimentRunCloseSheetWithTitle:@"Experiment ready"
                                                                 runAction:@"Run"];
                    }
                });
            }
        };
        core->registerCallback(mw::RESERVED_SYSTEM_EVENT_CODE, systemEventCallback, EVENT_CALLBACK_KEY);
        if (!core->openExperiment(path.UTF8String)) {
            core->unregisterCallbacks(EVENT_CALLBACK_KEY);
            completionHandler(NO);
        }
    } catch (const std::exception &e) {
        NSLog(@"Exception in %s: %s", __PRETTY_FUNCTION__, e.what());
    } catch (...) {
        NSLog(@"Unknown exception in %s", __PRETTY_FUNCTION__);
    }
}


- (void)openExperimentRunCloseSheetWithTitle:(NSString *)title runAction:(NSString *)runAction {
    try {
        UIViewController *viewController = UIApplication.sharedApplication.keyWindow.rootViewController;
        
        UIAlertController *alert = [UIAlertController alertControllerWithTitle:title
                                                                       message:nil
                                                                preferredStyle:UIAlertControllerStyleAlert];
        
        [alert addAction:[UIAlertAction actionWithTitle:runAction
                                                  style:UIAlertActionStyleDefault
                                                handler:^(UIAlertAction *action) {
                                                    [viewController dismissViewControllerAnimated:YES completion:nil];
                                                    [self runExperiment];
                                                }]];
        
        [alert addAction:[UIAlertAction actionWithTitle:@"Close"
                                                  style:UIAlertActionStyleDefault
                                                handler:^(UIAlertAction *action) {
                                                    [self closeExperiment];
                                                    [viewController dismissViewControllerAnimated:YES completion:nil];
                                                }]];
        
        [viewController presentViewController:alert animated:YES completion:nil];
    } catch (const std::exception &e) {
        NSLog(@"Exception in %s: %s", __PRETTY_FUNCTION__, e.what());
    } catch (...) {
        NSLog(@"Unknown exception in %s", __PRETTY_FUNCTION__);
    }
}


- (void)runExperiment {
    try {
        boost::weak_ptr<mw::Server> weakCore(core);
        auto systemEventCallback = [weakCore](const boost::shared_ptr<mw::Event> &event) {
            auto &data = event->getData();
            if (data.getElement(M_SYSTEM_PAYLOAD_TYPE).getInteger() == mw::M_EXPERIMENT_STATE) {
                auto &state = data.getElement(M_SYSTEM_PAYLOAD);
                if (state.getElement(M_LOADED).getBool() && !state.getElement(M_RUNNING).getBool()) {
                    if (auto core = weakCore.lock()) {
                        core->unregisterCallbacks(EVENT_CALLBACK_KEY);
                    }
                    dispatch_async(dispatch_get_main_queue(), ^{
                        [APP_DELEGATE openExperimentRunCloseSheetWithTitle:@"Experiment ended"
                                                                 runAction:@"Run again"];
                    });
                }
            }
        };
        core->registerCallback(mw::RESERVED_SYSTEM_EVENT_CODE, systemEventCallback, EVENT_CALLBACK_KEY);
        core->startExperiment();
    } catch (const std::exception &e) {
        NSLog(@"Exception in %s: %s", __PRETTY_FUNCTION__, e.what());
    } catch (...) {
        NSLog(@"Unknown exception in %s", __PRETTY_FUNCTION__);
    }
}


- (void)closeExperiment {
    try {
        core->closeExperiment();
    } catch (const std::exception &e) {
        NSLog(@"Exception in %s: %s", __PRETTY_FUNCTION__, e.what());
    } catch (...) {
        NSLog(@"Unknown exception in %s", __PRETTY_FUNCTION__);
    }
}


@end





























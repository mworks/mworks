//
//  AppDelegate.m
//  MWServer-iOS
//
//  Created by Christopher Stawarz on 2/17/17.
//
//

#import "AppDelegate.h"

#import "ViewController.h"

#define SERVER_NAME_PREFERENCE @"server_name_preference"
#define LISTENING_PORT_PREFERENCE @"listening_port_preference"
#define ALLOW_ALT_FAILOVER_PREFERENCE @"allow_alt_failover_preference"
#define STOP_ON_ERROR_PREFERENCE @"stop_on_error_preference"
#define WARN_ON_SKIPPED_REFRESH_PREFERENCE @"warn_on_skipped_refresh_preference"
#define HIDE_CHOOSE_EXPERIMENT_BUTTON_PREFERENCE @"hide_choose_experiment_button_preference"
#define DISPLAY_WIDTH_PREFERENCE @"display_width_preference"
#define DISPLAY_HEIGHT_PREFERENCE @"display_height_preference"
#define DISPLAY_DISTANCE_PREFERENCE @"display_distance_preference"
#define USE_COLOR_MANAGEMENT_PREFERENCE @"use_color_management_preference"
#define DID_INSTALL_EXAMPLE_EXPERIMENTS_PREFERENCE @"did_install_example_experiments_preference"

#define EVENT_CALLBACK_KEY @"<MWServer-iOS/AppDelegate>"
#define PERSISTENT_EVENT_CALLBACK_KEY @"<MWServer-iOS/AppDelegate-Persistent>"


@implementation AppDelegate {
    id<NSObject> ioActivity;
}


- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    self.viewController = (ViewController *)(self.window.rootViewController);
    self.viewController.appDelegate = self;
    
    // Ensure that Foundation is set up for multiple threads
    NSAssert(NSThread.isMultiThreaded, @"Foundation multithreading is not enabled");
    
    // Prevent system sleep
    application.idleTimerDisabled = YES;
    
    // Take a power assertion to prevent the OS from throttling long-running event listener and I/O threads.
    // (Not sure if this actually helps on iOS, but it probably can't hurt.)
    ioActivity = [NSProcessInfo.processInfo beginActivityWithOptions:(NSActivityBackground | NSActivityIdleSystemSleepDisabled)
                                                              reason:@"Prevent I/O throttling"];
    
    NSError *error = nil;
    if (![MWKCore constructCoreWithType:MWKCoreTypeServer error:&error]) {
        [self presentInitializationFailureAlertWithMessage:error.localizedDescription];
        return YES;
    }
    
    NSUserDefaults *userDefaults = NSUserDefaults.standardUserDefaults;
    registerDefaultSettings(userDefaults);
    self.setupVariablesController = [[MWKSetupVariablesController alloc] init];
    initializeSetupVariables(userDefaults, self.setupVariablesController);
    installExampleExperiments(userDefaults);
    
    self.listeningAddress = @"localhost";  // Temporary placeholder
    self.listeningPort = [userDefaults integerForKey:LISTENING_PORT_PREFERENCE];
    self.hideChooseExperimentButton = [userDefaults boolForKey:HIDE_CHOOSE_EXPERIMENT_BUTTON_PREFERENCE];
    
    // Determining the listening address may involve a slow DNS lookup, so do it on a dispatch queue
    // to avoid blocking the main thread
    dispatch_async(dispatch_get_global_queue(QOS_CLASS_USER_INTERACTIVE, 0), ^{
        self.listeningAddress = MWKServer.hostName;
        if (self.listeningAddress.length == 0) {
            // This is less reliable than MWKServer.hostName, so use it only as a fallback
            self.listeningAddress = NSProcessInfo.processInfo.hostName;
        }
        dispatch_async(dispatch_get_main_queue(), ^{
            self.viewController.listeningAddress.text = self.listeningAddress;
        });
    });
    
    // The first time the user launches the app, iOS will ask for permission to use the local network.
    // If serverWithListeningAddress:listeningPort:error: is running on the main thread, it will block
    // presentation of the permission dialog.  Therefore, we initialize the server on a dispatch queue.
    dispatch_async(dispatch_get_global_queue(QOS_CLASS_USER_INTERACTIVE, 0), ^{
        NSError *error = nil;
        self.server = [MWKServer serverWithListeningAddress:@""  // Bind to all addresses
                                              listeningPort:self.listeningPort
                                                      error:&error];
        if (!self.server) {
            [self presentInitializationFailureAlertWithMessage:error.localizedDescription];
            return;
        }
        
        [self.server start];
        
        AppDelegate * __weak weakSelf = self;
        MWKEventCallback systemEventCallback = ^(MWKEvent *event) {
            MWKDatum *data = event.data;
            if (data[MWKSystemEventKeyPayloadType].numberValue.integerValue == MWKSystemEventPayloadTypeExperimentState) {
                BOOL loaded = data[MWKSystemEventKeyPayload][MWKSystemEventPayloadKeyLoaded].numberValue.boolValue;
                dispatch_async(dispatch_get_main_queue(), ^{
                    weakSelf.viewController.chooseExperiment.enabled = !loaded;
                });
            }
        };
        [self.server registerCallbackWithKey:PERSISTENT_EVENT_CALLBACK_KEY
                                     forCode:MWKReservedEventCodeSystemEvent
                                    callback:systemEventCallback];
    });
    
    return YES;
}


static void registerDefaultSettings(NSUserDefaults *userDefaults) {
    NSDictionary *defaultSettings =
    @{
      LISTENING_PORT_PREFERENCE: @(19989),
      ALLOW_ALT_FAILOVER_PREFERENCE: @(YES),
      STOP_ON_ERROR_PREFERENCE: @(NO),
      WARN_ON_SKIPPED_REFRESH_PREFERENCE: @(YES),
      HIDE_CHOOSE_EXPERIMENT_BUTTON_PREFERENCE: @(NO),
      DISPLAY_WIDTH_PREFERENCE: @(373.33),
      DISPLAY_HEIGHT_PREFERENCE: @(280),
      DISPLAY_DISTANCE_PREFERENCE: @(450),
      USE_COLOR_MANAGEMENT_PREFERENCE: @(YES),
      DID_INSTALL_EXAMPLE_EXPERIMENTS_PREFERENCE: @(NO)
      };
    [userDefaults registerDefaults:defaultSettings];
}


static void initializeSetupVariables(NSUserDefaults *userDefaults,
                                     MWKSetupVariablesController *setupVariablesController)
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


- (void)applicationDidEnterBackground:(UIApplication *)application {
    [self.server stopExperiment];  // No-op if experiment is not loaded or not running
}


- (void)applicationWillTerminate:(UIApplication *)application {
    if (self.server) {
        [self.server unregisterCallbacksWithKey:PERSISTENT_EVENT_CALLBACK_KEY];
        [self.server stop];
    }
    
    // Release power assertion
    [NSProcessInfo.processInfo endActivity:ioActivity];
    
    // Re-enable system sleep
    application.idleTimerDisabled = NO;
}


- (void)presentInitializationFailureAlertWithMessage:(NSString *)message {
    UIAlertController* alert = [UIAlertController alertControllerWithTitle:@"Server initialization failed"
                                                                   message:message
                                                            preferredStyle:UIAlertControllerStyleAlert];
    dispatch_async(dispatch_get_main_queue(), ^{
        [self.viewController presentViewController:alert animated:YES completion:nil];
    });
}


- (void)openExperimentAtPath:(NSString *)path completionHandler:(void (^)(BOOL success))completionHandler {
    AppDelegate * __weak weakSelf = self;
    MWKEventCallback systemEventCallback = ^(MWKEvent *event) {
        MWKDatum *data = event.data;
        if (data[MWKSystemEventKeyPayloadType].numberValue.integerValue == MWKSystemEventPayloadTypeExperimentState) {
            [weakSelf.server unregisterCallbacksWithKey:EVENT_CALLBACK_KEY];
            BOOL success = data[MWKSystemEventKeyPayload][MWKSystemEventPayloadKeyLoaded].numberValue.boolValue;
            dispatch_async(dispatch_get_main_queue(), ^{
                completionHandler(success);
                if (success) {
                    [weakSelf openExperimentRunCloseSheetWithTitle:@"Experiment ready"
                                                         runAction:@"Run"];
                }
            });
        }
    };
    [self.server registerCallbackWithKey:EVENT_CALLBACK_KEY
                                 forCode:MWKReservedEventCodeSystemEvent
                                callback:systemEventCallback];
    
    if (![self.server openExperimentAtPath:path]) {
        [self.server unregisterCallbacksWithKey:EVENT_CALLBACK_KEY];
        completionHandler(NO);
    }
}


- (void)openExperimentRunCloseSheetWithTitle:(NSString *)title runAction:(NSString *)runAction {
    UIViewController *viewController =  nil;
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    for (UIWindow *window in UIApplication.sharedApplication.windows) {
#pragma clang diagnostic pop
        if (window.keyWindow) {
            viewController = window.rootViewController;
            break;
        }
    }
    NSAssert(viewController, @"Key window not found");
    
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
                                                [self.server closeExperiment];
                                                [viewController dismissViewControllerAnimated:YES completion:nil];
                                            }]];
    
    [viewController presentViewController:alert animated:YES completion:nil];
}


- (void)runExperiment {
    AppDelegate * __weak weakSelf = self;
    MWKEventCallback systemEventCallback = ^(MWKEvent *event) {
        MWKDatum *data = event.data;
        if (data[MWKSystemEventKeyPayloadType].numberValue.integerValue == MWKSystemEventPayloadTypeExperimentState) {
            MWKDatum *state = data[MWKSystemEventKeyPayload];
            if (state[MWKSystemEventPayloadKeyLoaded].numberValue.boolValue &&
                !(state[MWKSystemEventPayloadKeyRunning].numberValue.boolValue))
            {
                [weakSelf.server unregisterCallbacksWithKey:EVENT_CALLBACK_KEY];
                dispatch_async(dispatch_get_main_queue(), ^{
                    [weakSelf openExperimentRunCloseSheetWithTitle:@"Experiment ended"
                                                         runAction:@"Run again"];
                });
            }
        }
    };
    [self.server registerCallbackWithKey:EVENT_CALLBACK_KEY
                                 forCode:MWKReservedEventCodeSystemEvent
                                callback:systemEventCallback];
    
    [self.server startExperiment];
}


@end

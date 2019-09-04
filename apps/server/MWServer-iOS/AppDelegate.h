//
//  AppDelegate.h
//  MWServer-iOS
//
//  Created by Christopher Stawarz on 2/17/17.
//
//

#import <UIKit/UIKit.h>

#import <MWorksSwift/MWorksSwift.h>

#define APP_DELEGATE ((AppDelegate *)(UIApplication.sharedApplication.delegate))


@interface AppDelegate : UIResponder <UIApplicationDelegate>

@property (strong, nonatomic) UIWindow *window;
@property (strong, nonatomic) UIAlertController* alert;
@property (strong, nonatomic, readonly) MWKSetupVariablesController *setupVariablesController;
@property (strong, nonatomic, readonly) NSString *listeningAddress;
@property (strong, nonatomic, readonly) NSNumber *listeningPort;
@property (nonatomic, readonly) BOOL hideChooseExperimentButton;

- (void)openExperiment:(NSString *)path completionHandler:(void (^)(BOOL success))completionHandler;

@end


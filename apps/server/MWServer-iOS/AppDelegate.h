//
//  AppDelegate.h
//  MWServer-iOS
//
//  Created by Christopher Stawarz on 2/17/17.
//
//

@import UIKit;

@import MWorksSwift;


@class ViewController;


@interface AppDelegate : UIResponder <UIApplicationDelegate>

@property (weak, nonatomic) ViewController *viewController;

@property (strong, nonatomic) UIWindow *window;
@property (strong, nonatomic) MWKSetupVariablesController *setupVariablesController;
@property (strong, nonatomic) NSString *listeningAddress;
@property (nonatomic) NSInteger listeningPort;
@property (nonatomic) BOOL hideChooseExperimentButton;
@property (strong, nonatomic) MWKServer *server;

- (void)openExperimentAtPath:(NSString *)path completionHandler:(void (^)(BOOL success))completionHandler;

@end

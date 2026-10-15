//
//  SceneDelegate.h
//  MWServer-iOS
//
//  Created by Christopher Stawarz on 10/14/26.
//

@import UIKit;

@import MWorksSwift;


@class ViewController;


@interface SceneDelegate : UIResponder <UIWindowSceneDelegate>

@property (weak, nonatomic) ViewController *viewController;

@property (strong, nonatomic) UIWindow *window;
@property (strong, nonatomic) MWKSetupVariablesController *setupVariablesController;
@property (strong, nonatomic) NSString *listeningAddress;
@property (nonatomic) NSInteger listeningPort;
@property (nonatomic) BOOL hideChooseExperimentButton;
@property (strong, nonatomic) MWKServer *server;

- (void)openExperimentAtPath:(NSString *)path completionHandler:(void (^)(BOOL success))completionHandler;

@end

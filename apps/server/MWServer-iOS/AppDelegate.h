//
//  AppDelegate.h
//  MWServer-iOS
//
//  Created by Christopher Stawarz on 2/17/17.
//
//

#import <UIKit/UIKit.h>

#import "MWSSetupVariablesController.h"


@interface AppDelegate : UIResponder <UIApplicationDelegate>

@property (strong, nonatomic) UIWindow *window;
@property (strong, nonatomic) UIAlertController* alert;
@property (strong, nonatomic, readonly) MWSSetupVariablesController *setupVariablesController;
@property (strong, nonatomic, readonly) NSString *listeningAddress;
@property (strong, nonatomic, readonly) NSNumber *listeningPort;

@end


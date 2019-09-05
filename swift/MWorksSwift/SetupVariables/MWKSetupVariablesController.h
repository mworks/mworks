//
//  MWKSetupVariablesController.h
//  MWServer
//
//  Created by Christopher Stawarz on 6/4/14.
//
//

#import <Foundation/Foundation.h>


NS_ASSUME_NONNULL_BEGIN


NS_SWIFT_NAME(SetupVariablesController)
@interface MWKSetupVariablesController : NSObject

@property(nonatomic, copy) NSString *serverName;
@property(weak, nonatomic, readonly) NSArray *availableDisplays;
@property(nonatomic, strong) NSNumber *displayToUse;
@property(nonatomic, strong) NSNumber *displayWidth;
@property(nonatomic, strong) NSNumber *displayHeight;
@property(nonatomic, strong) NSNumber *displayDistance;
@property(nonatomic, strong) NSNumber *displayRefreshRateHz;
@property(nonatomic) BOOL alwaysDisplayMirrorWindow;
@property(nonatomic, strong) NSNumber *mirrorWindowBaseHeight;
@property(nonatomic) BOOL useColorManagement;
@property(nonatomic) BOOL setDisplayGamma;
@property(nonatomic, strong) NSNumber *redGamma;
@property(nonatomic, strong) NSNumber *greenGamma;
@property(nonatomic, strong) NSNumber *blueGamma;
@property(nonatomic) BOOL makeWindowOpaque;
@property(nonatomic) BOOL warnOnSkippedRefresh;
@property(nonatomic) BOOL stopOnError;
@property(nonatomic) BOOL allowAltFailover;

@end


NS_ASSUME_NONNULL_END

//
//  MWClientPlugin.h
//  MWorksCocoa
//
//  Created by Christopher Stawarz on 9/13/19.
//

#import <AppKit/AppKit.h>

#import <MWorksSwift/MWorksSwift.h>


NS_ASSUME_NONNULL_BEGIN


@protocol MWClientPluginWindowController <NSObject>

- (void)setInGroupedWindow:(BOOL)isInGroupedWindow;

@end


NS_SWIFT_NAME(ClientPluginWorkspaceState)
@protocol MWClientPluginWorkspaceState <NSObject>

- (NSDictionary *)workspaceState;
- (void)setWorkspaceState:(NSDictionary *)workspaceState;

@end


NS_SWIFT_NAME(ClientPluginViewControllerFactory)
@protocol MWClientPluginViewControllerFactory <NSObject>

+ (NSViewController *)viewControllerWithClient:(id<MWKClient>)client NS_SWIFT_NAME(viewController(client:));

@end


NS_ASSUME_NONNULL_END

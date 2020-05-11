//
//  MWClientPlugin.h
//  MWorksCocoa
//
//  Created by Christopher Stawarz on 9/13/19.
//

#import <Foundation/Foundation.h>

#import <MWorksSwift/MWorksSwift.h>


@protocol MWClientPluginHost <NSObject>

@property(nonatomic, readonly) id<MWKClient> clientCore;

@end


@protocol MWClientPluginWindowController <NSObject>

- (void)setInGroupedWindow:(BOOL)isInGroupedWindow;

@end


@protocol MWClientPluginWorkspaceState <NSObject>

- (NSDictionary *)workspaceState;
- (void)setWorkspaceState:(NSDictionary *)workspaceState;

@end

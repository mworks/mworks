//
//  MWClientPlugin.h
//  MWorksCocoa
//
//  Created by Christopher Stawarz on 9/13/19.
//

#import <Foundation/Foundation.h>


@protocol MWClientPluginWindowController <NSObject>

- (void)setInGroupedWindow:(BOOL)isInGroupedWindow;

@end


@protocol MWClientPluginWorkspaceState <NSObject>

- (NSDictionary *)workspaceState;
- (void)setWorkspaceState:(NSDictionary *)workspaceState;

@end

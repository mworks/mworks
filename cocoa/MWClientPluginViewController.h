//
//  MWClientPluginViewController.h
//  MWorksCocoa
//
//  Created by Christopher Stawarz on 5/26/20.
//

#import <AppKit/AppKit.h>

#import <MWorksCocoa/MWClientPlugin.h>


NS_ASSUME_NONNULL_BEGIN


@interface MWClientPluginViewController : NSViewController <MWClientPluginViewControllerFactory, MWClientPluginWorkspaceState>

- (instancetype)initWithClient:(id<MWKClient>)client;

@property(nonatomic, weak, readonly) id<MWKClient> client;

- (id<NSObject>)registerStoredPropertyWithName:(NSString *)name
                                   defaultsKey:(NSString *)defaultsKey;

- (id<NSObject>)registerStoredPropertyWithName:(NSString *)name
                                   defaultsKey:(NSString *)defaultsKey
                                  workspaceKey:(NSString *)workspaceKey;

- (id<NSObject>)registerStoredPropertyWithName:(NSString *)name
                                   defaultsKey:(NSString *)defaultsKey
                                  handleEvents:(BOOL)handleEvents;

- (id<NSObject>)registerStoredPropertyWithName:(NSString *)name
                                   defaultsKey:(NSString *)defaultsKey
                                  workspaceKey:(NSString *)workspaceKey
                                  handleEvents:(BOOL)handleEvents;

- (BOOL)validateWorkspaceValue:(id)value forStoredProperty:(id<NSObject>)property;
- (void)handleEvent:(MWKEvent *)event forStoredProperty:(id<NSObject>)property;

- (void)postMessage:(NSString *)message;
- (void)postWarning:(NSString *)warning;
- (void)postError:(NSString *)error;

@end


NS_ASSUME_NONNULL_END

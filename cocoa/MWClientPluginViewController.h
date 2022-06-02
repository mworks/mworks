//
//  MWClientPluginViewController.h
//  MWorksCocoa
//
//  Created by Christopher Stawarz on 5/26/20.
//

#import <AppKit/AppKit.h>

#import <MWorksCocoa/MWClientPlugin.h>


NS_ASSUME_NONNULL_BEGIN


@interface MWClientPluginViewController : NSViewController <MWClientPluginViewControllerFactory>

- (instancetype)initWithClient:(id<MWKClient>)client;

@property(nonatomic, weak, readonly) id<MWKClient> client;

@end


NS_ASSUME_NONNULL_END

//
//  MWClientBase.h
//  MWorksCocoa
//
//  Created by Christopher Stawarz on 9/13/19.
//

#import <Cocoa/Cocoa.h>

#import <MWorksSwift/MWorksSwift.h>

#import <MWorksCocoa/MWClientServerBase.h>
#import <MWorksCocoa/MWNotebook.h>


@class MWCodec;


NS_ASSUME_NONNULL_BEGIN


@interface MWClientBase : MWClientServerBase

- (instancetype)initWithClient:(MWKClient *)client;

@property(nonatomic, readonly) MWKClient *client;

@property(nonatomic, readonly) MWNotebook *notebook;
@property(nonatomic, readonly) MWCodec *variables;
@property(nonatomic, readonly) NSArray *variableNames;
@property(nullable, nonatomic, strong) NSWindow *groupedPluginWindow;

@end


NS_ASSUME_NONNULL_END

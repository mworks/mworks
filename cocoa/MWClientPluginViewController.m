//
//  MWClientPluginViewController.m
//  MWorksCocoa
//
//  Created by Christopher Stawarz on 5/26/20.
//

#import "MWClientPluginViewController.h"


@implementation MWClientPluginViewController


+ (NSViewController *)viewControllerWithClient:(id<MWKClient>)client {
    return [[self alloc] initWithClient:client];
}


- (instancetype)initWithClient:(id<MWKClient>)client {
    Class class = [self class];
    self = [super initWithNibName:NSStringFromClass(class) bundle:[NSBundle bundleForClass:class]];
    if (self) {
        _client = client;
    }
    return self;
}


@end

//
//  MWEyeSamplePlotElement.h
//  MWorksEyeWindow
//
//  Created by David Cox on 2/3/06.
//  Copyright 2006 MIT. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface MWEyeSamplePlotElement : NSObject

@property(readonly) NSPoint position;
@property(readonly) NSTimeInterval time;
@property(readonly, getter=isSaccading) BOOL saccading;

- (instancetype)initWithTime:(NSTimeInterval)time position:(NSPoint)position;
- (instancetype)initWithTime:(NSTimeInterval)time position:(NSPoint)position saccading:(BOOL)saccading;

@end

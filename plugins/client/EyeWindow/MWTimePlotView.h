//
//  MWTimePlotView.h
//  MWorksEyeWindow
//
//  Created by Christopher Stawarz on 7/17/14.
//
//

#import <Cocoa/Cocoa.h>

#import <MWorksCocoa/MWCocoaEvent.h>


@interface MWTimePlotView : NSView

@property(nonatomic, copy) NSArray *eyeSamples;
@property(nonatomic, copy) NSArray *auxSamples;
@property(nonatomic) NSRect positionBounds;
@property(nonatomic) NSTimeInterval timeInterval;

- (void)addAEvent:(MWCocoaEvent *)event;
- (void)addBEvent:(MWCocoaEvent *)event;

- (void)reset;

@end

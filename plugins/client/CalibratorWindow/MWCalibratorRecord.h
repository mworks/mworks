//
//  CalibratorRecord.h
//  MWorksCalibratorWindow
//
//  Created by bkennedy on 9/12/06.
//  Copyright 2006 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface MWCalibratorRecord : NSObject {
	NSString *name;

	NSMutableArray *hParameters;
	NSMutableArray *vParameters;
}
- (id)initWithCalibratorName:(NSString *)calibratorName maxHParams:(int)maxHParams maxVParams:(int)maxVParams;
- (NSString *)getCalibratorName;
- (int)getNHParameters;
- (int)getNVParameters;
- (double)getHParameter:(int)index;
- (double)getVParameter:(int)index;
- (void)setHParameterIndex:(int)index value:(double)value;
- (void)setVParameterIndex:(int)index value:(double)value;

@end

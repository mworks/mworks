//
//  CalibratorRecord.m
//  MWorksCalibratorWindow
//
//  Created by bkennedy on 9/12/06.
//  Copyright 2006 __MyCompanyName__. All rights reserved.
//

#import "MWCalibratorRecord.h"

@implementation MWCalibratorRecord

- (id)initWithCalibratorName:(NSString *)calibratorName maxHParams:(int)maxHParams maxVParams:(int)maxVParams {
	name = [[NSString alloc] initWithString:calibratorName];
	
	hParameters = [[NSMutableArray alloc] init];
	vParameters = [[NSMutableArray alloc] init];
	
	for(int i=0; i<maxHParams; ++i) {
		[hParameters insertObject:[NSNumber numberWithFloat:0] atIndex:i];
	}
	
	for(int i=0; i<maxVParams; ++i) {
		[vParameters insertObject:[NSNumber numberWithFloat:0] atIndex:i];
	}
	
	self = [super init];
	return self;
}


- (NSString *)getCalibratorName {
	return name;
}

- (int)getNHParameters {
	return [hParameters count];
}

- (int)getNVParameters {
	return [vParameters count];
}

- (double)getHParameter:(int)index {
	if(index < [hParameters count]) 
		return [[hParameters objectAtIndex: index ] doubleValue];

	return 0;
}

- (double)getVParameter:(int)index {
	if(index < [vParameters count]) 
		return [[vParameters objectAtIndex: index ] doubleValue];

	return 0;
}

- (void)setHParameterIndex:(int)index value:(double)value {
	if(index < [hParameters count]) 
		[hParameters replaceObjectAtIndex:index withObject:[NSNumber numberWithFloat:value ]];
}

- (void)setVParameterIndex:(int)index value:(double)value {
	if(index < [vParameters count]) 
		[vParameters replaceObjectAtIndex:index withObject:[NSNumber numberWithFloat:value ]];
}

@end

//
//  MWStimulusPlotElement.m
//  MWorksEyeWindow
//
//  Created by Nuo Li on 8/9/06.
//  Copyright 2006 __MyCompanyName__. All rights reserved.
//

#import "MWStimulusPlotElement.h"
#import "MWorksCore/StandardVariables.h"


@implementation MWStimulusPlotElement


- (id)initStimElement:(NSString *)type
                 Name:(NSString *)name
                  AtX:(float)pos_x
                  AtY:(float)pos_y
               WidthX:(float)size_x
               WidthY:(float)size_y
             Rotation:(float)rot
{
	self = [super init];
	if (self) {
		stm_name = [name copy];
		stm_type = [type copy];
		stm_isOn = YES;
		center = NSMakePoint(pos_x, pos_y);
		size = NSMakeSize(size_x, size_y);
        rotation = rot;
	}
	return self;
}


- (NSString *)getName {
	return stm_name;
}
	
	
- (NSString *)getType {
	return stm_type;
}
	
	
- (BOOL)onOff {
	return stm_isOn;
}
	
- (void)setOnOff: (BOOL)on {
	stm_isOn = on;
}

- (void)setPositionX: (float)pos_x {
	center = NSMakePoint(pos_x, center.y);
}


- (void)setPositionY: (float)pos_y {
	center = NSMakePoint(center.x, pos_y);
}


- (void)setSizeX: (float)size_x {
	size = NSMakeSize(size_x, size.height);
}


- (void)setSizeY: (float)size_y {
	size = NSMakeSize(size.width, size_y);
}

- (NSBezierPath *)pathForBox {
    NSBezierPath *path = [NSBezierPath bezierPath];
    NSRect rect = NSMakeRect(-(size.width/2), -(size.height/2), size.width, size.height);
    
    if ([stm_type isEqualToString:@"circle"]) {
        [path appendBezierPathWithOvalInRect:rect];
    } else {
        [path appendBezierPathWithRect:rect];
    }
    
    NSAffineTransform *transform = [NSAffineTransform transform];
    [transform translateXBy:center.x yBy:center.y];
    [transform rotateByDegrees:rotation];
    [path transformUsingAffineTransform:transform];
    
    return path;
}

- (NSBezierPath *)pathForCrossWithSize:(NSSize)draw_size {
    NSBezierPath *path = [NSBezierPath bezierPath];
    
    [path moveToPoint:NSMakePoint(center.x-draw_size.width/2, center.y)];
    [path lineToPoint:NSMakePoint(center.x+draw_size.width/2, center.y)];
    
    [path moveToPoint:NSMakePoint(center.x, center.y-draw_size.height/2)];
    [path lineToPoint:NSMakePoint(center.x, center.y+draw_size.height/2)];
    
    return path;
}


//========================= Drawing the stimulus ====================================
- (void)stroke:(NSRect)visible degreesToPoints:(NSAffineTransform *)degreesToPoints {
	if (stm_isOn) {
        NSBezierPath *path = [NSBezierPath bezierPath];
        
		if ([stm_type isEqualToString:@STIM_TYPE_POINT]) {
            
            [[NSColor greenColor] set];
            
            // Don't use pathForBox, because, as currently implemented, the fixation window is never
            // rotated, even if the visible fixation rectangle is
            [path appendBezierPath:[NSBezierPath bezierPathWithRect:NSMakeRect(center.x-(size.width/2),
                                                                               center.y-(size.height/2),
                                                                               size.width,
                                                                               size.height)]];
            
            [path appendBezierPath:[self pathForCrossWithSize:NSMakeSize(0.3*size.width, 0.3*size.height)]];
            
        } else if ([stm_type isEqualToString:@STIM_TYPE_CIRCULAR_FIXATION_POINT]) {
            
            [[NSColor greenColor] set];
            
            [path appendBezierPath:[NSBezierPath bezierPathWithOvalInRect:NSMakeRect(center.x-(size.width/2),
                                                                                     center.y-(size.height/2),
                                                                                     size.width,
                                                                                     size.height)]];
            
            [path appendBezierPath:[self pathForCrossWithSize:NSMakeSize(0.3*size.width, 0.3*size.height)]];
            
		} else if ([stm_type isEqualToString:@"calibratorSample"]) {
            
            [[NSColor redColor] set];
			
			const float largest_visible_dimension = MAX(visible.size.width, visible.size.height);
            [path appendBezierPath:[self pathForCrossWithSize:NSMakeSize(0.03*largest_visible_dimension,
                                                                         0.03*largest_visible_dimension)]];
			
			[NSTimer scheduledTimerWithTimeInterval: 0.1
				target: self
				selector: @selector(eraseCalSample:)
				userInfo: nil
				repeats: NO];
            
		} else {
            
            [[NSColor redColor] set];
            [path appendBezierPath:[self pathForBox]];
            
        }
        
        [path transformUsingAffineTransform:degreesToPoints];
        [path stroke];
	}
}
//=====================================================================================

-(void) eraseCalSample:(NSTimer *)theTimer {
	[self setOnOff:NO];
}


@end




























//#import "MWorksCore/InterfaceSetting.h"
#import "MWPlotView.h"
#import "MWEyeSamplePlotElement.h"
#import "MWStimulusPlotElement.h"
#import "MWorksCore/StandardVariables.h"

#define PLOT_VIEW_FULL_SIZE	900.0f
#define MAX_ANGLE	180.0f
#define SCALE_SLIDER_WIDTH 100.0f

#define FIXATION 0
#define SACCADING 1


@interface MWPlotView ()

@property(nonatomic, strong) MWCocoaEvent *currentEyeH;
@property(nonatomic, strong) MWCocoaEvent *currentEyeV;

@property(nonatomic, strong) MWCocoaEvent *currentAuxH;
@property(nonatomic, strong) MWCocoaEvent *currentAuxV;

@end


@implementation MWPlotView {
    dispatch_queue_t serialQueue;
    BOOL updatePending;
    
    CGFloat degreesPerPoint;
    NSPoint frameCenterDegrees;
    
    NSPoint lastDragLocation;
}

@synthesize currentEyeH, currentEyeV;


- (id)initWithFrame:(NSRect)frameRect {
    if ((self = [super initWithFrame:frameRect])) {
        // This should be a layer-backed view, as the drawing performance of non-layer-backed views is
        // terrible under macOS 10.13
        self.wantsLayer = YES;
        
        serialQueue = dispatch_queue_create(NULL, DISPATCH_QUEUE_SERIAL);
        
        width = MAX_ANGLE;
        gridStepX = 10;
        gridStepY = 10;
        cartesianGrid = YES;
        
        eye_samples = [[NSMutableArray alloc] init];
        aux_samples = [[NSMutableArray alloc] init];
        stm_samples = [[NSMutableArray alloc] init];
        cal_samples = [[NSMutableArray alloc] init];
        
        last_state_change_time = 0;
        current_state = FIXATION;
        
        // these correspond to the defaults in the options window.
        timeOfTail = 1.0; // 1s
        
        updatePending = NO;
        
        degreesPerPoint = MAX_ANGLE / PLOT_VIEW_FULL_SIZE;
        frameCenterDegrees = NSMakePoint(0, 0);
	}
    
	return self;
}


- (NSAffineTransform *)pointsToDegrees {
    NSAffineTransform *pointsToDegrees = [NSAffineTransform transform];
    NSRect bounds = [self bounds];
    
    [pointsToDegrees translateXBy:(frameCenterDegrees.x - (NSWidth(bounds)  * degreesPerPoint / 2))
                              yBy:(frameCenterDegrees.y - (NSHeight(bounds) * degreesPerPoint / 2))];
    [pointsToDegrees scaleBy:degreesPerPoint];
    
    return pointsToDegrees;
}


static void removeExpiredSamples(NSMutableArray *samples, NSTimeInterval cutoffTime) {
    NSUInteger firstValidIndex = [samples indexOfObjectPassingTest:^BOOL (id obj, NSUInteger idx, BOOL *stop) {
        MWEyeSamplePlotElement *sample = obj;
        return (sample.time >= cutoffTime);
    }];
    
    if (firstValidIndex != NSNotFound) {
        [samples removeObjectsInRange:NSMakeRange(0, firstValidIndex)];
    } else {
        // All samples have expired
        [samples removeAllObjects];
    }
}


- (void)viewWillDraw {
	dispatch_sync(serialQueue, ^{
        NSRect bounds = [self bounds];
        NSAffineTransform *pointsToDegrees = [self pointsToDegrees];
		
		NSRect visible;
        visible.origin = [pointsToDegrees transformPoint:bounds.origin];
        visible.size = [pointsToDegrees transformSize:bounds.size];
        
        NSTimeInterval currentTime = [NSDate timeIntervalSinceReferenceDate];
        NSTimeInterval cutoffTime = currentTime - timeOfTail;
        
        removeExpiredSamples(eye_samples, cutoffTime);
        removeExpiredSamples(aux_samples, cutoffTime);
        
        // TODO: Can we just remove this?
        /*
		if ([eye_samples count]) {
            //
            // Schedule the next check for expired samples to occur at the first sample's expiration time.
            // We don't need to worry about timeOfTail getting smaller, because setTimeOfTail: always
            // triggers an update.
            //
            NSTimeInterval firstSampleExpirationTime = ((MWEyeSamplePlotElement *)[eye_samples objectAtIndex:0]).time + timeOfTail;
            dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (firstSampleExpirationTime - currentTime) * NSEC_PER_SEC),
                           serialQueue,
                           ^{
                               NSTimeInterval cutoffTime = [NSDate timeIntervalSinceReferenceDate] - timeOfTail;
                               if (([eye_samples count] > 0) && ([(MWEyeSamplePlotElement *)[eye_samples objectAtIndex:0] time] < cutoffTime)) {
                                   [self triggerUpdate];
                               }
                           });
		}
         */
		
        // Update the time plot panel
        self.timePlot.eyeSamples = [eye_samples copy];
        self.timePlot.auxSamples = [aux_samples copy];
        self.timePlot.positionBounds = visible;
        self.timePlot.timeInterval = timeOfTail;
        
        updatePending = NO;
	});
    
    [self.timePlot setNeedsDisplay:YES];
    
    [super viewWillDraw];
}


- (void)drawRect:(NSRect)rect {
	dispatch_sync(serialQueue, ^{
        [[NSGraphicsContext currentContext] setShouldAntialias:NO];
        [NSBezierPath setDefaultLineWidth:0.0];  // Draw lines as thin as possible
        
        NSRect bounds = [self bounds];
        
        NSAffineTransform *pointsToDegrees = [self pointsToDegrees];
        NSAffineTransform *degreesToPoints = [pointsToDegrees copy];
        [degreesToPoints invert];
		
		NSRect visible;
        visible.origin = [pointsToDegrees transformPoint:bounds.origin];
        visible.size = [pointsToDegrees transformSize:bounds.size];
        
        // Background
        {
            [[NSColor textBackgroundColor] set];
            NSRectFill(bounds);
        }
        
        // Border
        {
            [[NSColor grayColor] set];
            NSFrameRect(bounds);
        }
        
        // Grid lines
        {
            NSBezierPath *grid = [NSBezierPath bezierPath];
            
            const float lowest_y_draw = 10*round(visible.origin.y/10);
            for(float y_pos=lowest_y_draw; y_pos<visible.origin.y+visible.size.height; y_pos+=gridStepY) {
                [grid moveToPoint:NSMakePoint(NSMinX(visible), y_pos)];
                [grid lineToPoint:NSMakePoint(NSMaxX(visible), y_pos)];
            }
            
            const float lowest_x_draw = 10*round(visible.origin.x/10);
            for(float x_pos=lowest_x_draw; x_pos<visible.origin.x+visible.size.width; x_pos+=gridStepX) {
                [grid moveToPoint:NSMakePoint(x_pos, NSMinY(visible))];
                [grid lineToPoint:NSMakePoint(x_pos, NSMaxY(visible))];
            }
            
            [grid transformUsingAffineTransform:degreesToPoints];
            
            [[NSColor grayColor] set];
            [grid stroke];
        }
        
        // Display outline
        {
            NSBezierPath *outline = [NSBezierPath bezierPathWithRect:displayBounds];
            [outline transformUsingAffineTransform:degreesToPoints];
            
            [[NSColor textColor] set];
            [outline setLineWidth:1];
            [outline stroke];
        }

		if([eye_samples count]) {
			MWEyeSamplePlotElement *last_sample = [eye_samples objectAtIndex:0];
            NSPoint lastPos = [degreesToPoints transformPoint:last_sample.position];
			
            NSBezierPath *saccadePath = [NSBezierPath bezierPath];
            [[NSColor textColor] set];
            
			for(int i = 1; i < [eye_samples count]-1; ++i) {
				MWEyeSamplePlotElement *current_sample = [eye_samples objectAtIndex:i];
                NSPoint currentPos = [degreesToPoints transformPoint:current_sample.position];
				
				if(last_sample.saccading == SACCADING || current_sample.saccading == SACCADING) {
                    [saccadePath moveToPoint:lastPos];
                    [saccadePath lineToPoint:currentPos];
				}
				
				if(current_sample.saccading == FIXATION) {
                    NSRectFill(NSMakeRect(currentPos.x - 0.5, currentPos.y - 0.5, 1, 1));
				}
                
                last_sample = current_sample;
                lastPos = currentPos;
			}
            
            if (![saccadePath isEmpty]) {
                [[NSColor colorWithCalibratedRed:0.0 green:0.588 blue:1.0 alpha:1.0] set];
                [saccadePath stroke];
            }
		}
        
		if ([aux_samples count]) {
            NSBezierPath *path = [NSBezierPath bezierPath];
            
            {
                MWEyeSamplePlotElement *firstSample = [aux_samples objectAtIndex:0];
                [path moveToPoint:[degreesToPoints transformPoint:firstSample.position]];
            }
			
			for (int i = 1; i < [aux_samples count]-1; i++) {
				MWEyeSamplePlotElement *sample = [aux_samples objectAtIndex:i];
                [path lineToPoint:[degreesToPoints transformPoint:sample.position]];
			}
            
            [[NSColor cyanColor] set];
            [path stroke];
		}
		
		//======================= Draws stimulus ==================================
		// Goes through the NSMutable array 'stm_samples' to display each item in 
		// the array
		for (MWStimulusPlotElement *stimulus in stm_samples) {
			[stimulus stroke:visible degreesToPoints:degreesToPoints];
		}
		for (MWStimulusPlotElement *cal in cal_samples) {
			[cal stroke:visible degreesToPoints:degreesToPoints];
		}
        
        if ([eye_samples count] || [aux_samples count]) {
            [self triggerUpdate];
        }
	});
}


- (BOOL)acceptsFirstMouse:(NSEvent *)event {
    return YES;
}


- (void)mouseDown:(NSEvent *)event {
    lastDragLocation = [self convertPoint:[event locationInWindow] fromView:nil];
    [[NSCursor closedHandCursor] push];
}


- (void)mouseDragged:(NSEvent *)event {
    NSPoint dragLocation = [self convertPoint:[event locationInWindow] fromView:nil];
    NSSize displacement = NSMakeSize(dragLocation.x - lastDragLocation.x,
                                     dragLocation.y - lastDragLocation.y);
    
    NSAffineTransform *pointsToDegrees = [self pointsToDegrees];
    NSSize displacementDegrees = [pointsToDegrees transformSize:displacement];
    
    frameCenterDegrees.x -= displacementDegrees.width;
    frameCenterDegrees.y -= displacementDegrees.height;
    
    lastDragLocation = dragLocation;
    [self setNeedsDisplay:YES];
}


- (void)mouseUp:(NSEvent *)event {
    [NSCursor pop];
}


- (void)setDisplayBounds:(NSRect)bounds {
	dispatch_async(serialQueue, ^{
		displayBounds = bounds;
        [self triggerUpdate];
	});
}


- (void)setWidth:(float)width_in {
    // This method should never be called from a non-main thread
    NSAssert1([NSThread isMainThread], @"%s called on non-main thread", __func__);
    
    if (width_in != width) {
        width = width_in;
        degreesPerPoint = width / PLOT_VIEW_FULL_SIZE;
        
        // \xc2\xb0 is the UTF-8 encoding of the degree symbol (¡)
        [self.scaleLabel setStringValue:[NSString stringWithFormat:@"%.1f\xc2\xb0", SCALE_SLIDER_WIDTH * degreesPerPoint]];
        
        // Call setNeedsDisplay: directly, instead of invoking triggerUpdate, because we're already on the
        // main thread, and we don't want to introduce delays by dispatching to serialQueue
        [self setNeedsDisplay:YES];
    }
}

- (void)addEyeHEvent:(MWCocoaEvent *)event {
	dispatch_async(serialQueue, ^{
        if (!self.currentEyeH || ([event time] > [self.currentEyeH time])) {
            [self syncHEvent:event withVEvent:self.currentEyeV];
        }
	});
}

- (void)addEyeVEvent:(MWCocoaEvent *)event {
	dispatch_async(serialQueue, ^{
        if (!self.currentEyeV || ([event time] > [self.currentEyeV time])) {
            [self syncHEvent:self.currentEyeH withVEvent:event];
        }
	});
}

#define EVENT_SYNC_TIME_US 250

- (void)syncHEvent:(MWCocoaEvent *)eyeH withVEvent:(MWCocoaEvent *)eyeV {
    MWorksTime eyeHTime;
    MWorksTime eyeVTime;
    BOOL synced = NO;

    if (eyeH && eyeV) {
        eyeHTime = [eyeH time];
        eyeVTime = [eyeV time];
        MWorksTime t_diff = eyeHTime - eyeVTime;
        if (std::abs(t_diff) <= EVENT_SYNC_TIME_US) {
            synced = YES;
        } else {
            if (t_diff > 0) {
                eyeV = nil;
            } else {
                eyeH = nil;
            }
        }
    }
    
    self.currentEyeH = eyeH;
    self.currentEyeV = eyeV;
    
    if (synced) {
        //
        // We have a synced eyeH/eyeV pair, so add a plot element for it
        //
        
        int eye_state = (std::max(eyeHTime, eyeVTime) >= last_state_change_time) ? current_state : !current_state;
        
        MWEyeSamplePlotElement *sample = nil;
        sample = [[MWEyeSamplePlotElement alloc] initWithTime:[NSDate timeIntervalSinceReferenceDate]
                                                     position:NSMakePoint([self.currentEyeH data]->getFloat(),
                                                                          [self.currentEyeV data]->getFloat()) 
                                                  saccading:eye_state];
        [eye_samples addObject:sample];
        
        self.currentEyeH = nil;
        self.currentEyeV = nil;
        [self triggerUpdate];
    }
}

- (void)addEyeStateEvent:(MWCocoaEvent *)event {
	dispatch_async(serialQueue, ^{
		if([event data]->getInteger() != current_state) {
			MWorksTime time_of_state_change = [event time];
			if(time_of_state_change > last_state_change_time) {
				last_state_change_time = time_of_state_change;
				current_state = !current_state;
			}
		}
	});
}


- (void)addAuxHEvent:(MWCocoaEvent *)event {
	dispatch_async(serialQueue, ^{
        if (!self.currentAuxH || ([event time] > [self.currentAuxH time])) {
            [self syncAuxHEvent:event withAuxVEvent:self.currentAuxV];
        }
	});
}


- (void)addAuxVEvent:(MWCocoaEvent *)event {
	dispatch_async(serialQueue, ^{
        if (!self.currentAuxV || ([event time] > [self.currentAuxV time])) {
            [self syncAuxHEvent:self.currentAuxH withAuxVEvent:event];
        }
	});
}


- (void)syncAuxHEvent:(MWCocoaEvent *)auxH withAuxVEvent:(MWCocoaEvent *)auxV {
    MWorksTime auxHTime;
    MWorksTime auxVTime;
    BOOL synced = NO;
    
    if (auxH && auxV) {
        auxHTime = [auxH time];
        auxVTime = [auxV time];
        MWorksTime t_diff = auxHTime - auxVTime;
        // TODO: Figure this out!
        if (std::abs(t_diff) <= 500000) {
            synced = YES;
        } else {
            if (t_diff > 0) {
                auxV = nil;
            } else {
                auxH = nil;
            }
        }
    }
    
    self.currentAuxH = auxH;
    self.currentAuxV = auxV;
    
    if (synced) {
        //
        // We have a synced auxH/auxV pair, so add a plot element for it
        //
        
        MWEyeSamplePlotElement *sample = nil;
        sample = [[MWEyeSamplePlotElement alloc] initWithTime:[NSDate timeIntervalSinceReferenceDate]
                                                     position:NSMakePoint([self.currentAuxH data]->getFloat(),
                                                                          [self.currentAuxV data]->getFloat())];
        [aux_samples addObject:sample];
        
        self.currentAuxH = nil;
        self.currentAuxV = nil;
        [self triggerUpdate];
    }
}


//==================== stimulus announce is handled here ===============================
- (void)acceptStmAnnounce:(mw::Datum *)_stm_announce_list Time:(MWorksTime)event_time
{
    // Need a copy for async usage
    mw::Datum stm_announce_list = *_stm_announce_list;
    
	dispatch_async(serialQueue, ^{
        [stm_samples removeAllObjects];
        
        const int numStims = stm_announce_list.getNElements();
        for (int i =  0; i < numStims; i++) {
            mw::Datum stm_announce = stm_announce_list.getElement(i);
            
            if (!stm_announce.isDictionary()) {
                continue;
            }
            
            // Handle movies
            {
                mw::Datum current_stimulus = stm_announce.getElement("current_stimulus");
                if (current_stimulus.isDictionary()) {
                    stm_announce = current_stimulus;
                }
            }
            
            mw::Datum type_data = stm_announce.getElement(STIM_TYPE);
            mw::Datum name_data = stm_announce.getElement(STIM_NAME);
            mw::Datum pos_x_data = stm_announce.getElement(STIM_POSX);
            mw::Datum pos_y_data = stm_announce.getElement(STIM_POSY);
            mw::Datum width_x_data = stm_announce.getElement(STIM_SIZEX);
            mw::Datum width_y_data = stm_announce.getElement(STIM_SIZEY);
            mw::Datum rotation_data = stm_announce.getElement(STIM_ROT);
            
            mw::Datum fullscreen_data = stm_announce.getElement(STIM_FULLSCREEN);
            if (fullscreen_data.isNumber() && fullscreen_data.getBool()) {
                // For fullscreen stimuli, the bounding box is the entire display
                pos_x_data.setFloat(0.0);
                pos_y_data.setFloat(0.0);
                width_x_data.setFloat(displayBounds.size.width);
                width_y_data.setFloat(displayBounds.size.height);
                rotation_data.setFloat(0.0);
            }
            
            if (type_data.isString() &&
                name_data.isString() &&
                pos_x_data.isNumber() &&
                pos_y_data.isNumber() &&
                width_x_data.isNumber() &&
                width_y_data.isNumber() &&
                rotation_data.isNumber())
            {
                if (type_data == STIM_TYPE_POINT || type_data == STIM_TYPE_CIRCULAR_FIXATION_POINT) {
                    // For fixation points, we want to display the trigger area, not the visible rectangle
                    width_x_data = width_y_data = stm_announce.getElement("width");
                }
                
                NSString* stm_type = @(type_data.getString().c_str());
                NSString* stm_name = @(name_data.getString().c_str());
                float stm_pos_x = pos_x_data.getFloat();
                float stm_pos_y = pos_y_data.getFloat();
                float stm_width_x = width_x_data.getFloat();
                float stm_width_y = width_y_data.getFloat();
                float rotation = rotation_data.getFloat();
                
                MWStimulusPlotElement *new_stm = [[MWStimulusPlotElement alloc] initStimElement:stm_type
                                                                                           Name:stm_name
                                                                                            AtX:stm_pos_x
                                                                                            AtY:stm_pos_y
                                                                                         WidthX:stm_width_x
                                                                                         WidthY:stm_width_y
                                                                                       Rotation:rotation];
                [stm_samples addObject:new_stm];
            }
        }
        
        [self triggerUpdate];
	});
}
//=====================================================================================






//==================== calibrator announce is handled here ===============================
- (void)acceptCalAnnounce:(mw::Datum *)_cal_announce
{
    // Need a copy for async usage
    mw::Datum cal_announce = *_cal_announce;
    
	dispatch_async(serialQueue, ^{
		NSString* stm_name = @"calibrator";
		NSString *stm_type = @"calibratorSample";
		
		//Check calibrator action first
		mw::Datum actionData = cal_announce.getElement(CALIBRATOR_ACTION);
		mw::Datum cal_sample_HV = cal_announce.getElement(CALIBRATOR_SAMPLE_CALIBRATED_HV);
		
		if(actionData.isString() && cal_sample_HV.isList()) {
			if (actionData == CALIBRATOR_ACTION_SAMPLE && cal_sample_HV.getNElements() == 2) {
				float cal_sample_H = (cal_sample_HV.getElement(0)).getFloat();
				float cal_sample_V = (cal_sample_HV.getElement(1)).getFloat();
				
				
				// Checking to see if the item is already in the list
				BOOL Item_exist = NO;
				int i;
				
				for(i = 0; i < [cal_samples count]; i++) {
					MWStimulusPlotElement *existing_stm = [cal_samples objectAtIndex:i];
					if ([[existing_stm getName] isEqualToString: stm_name]) {
						
						[existing_stm setOnOff:YES];
						[existing_stm setPositionX:cal_sample_H];
						[existing_stm setPositionY:cal_sample_V];
						[existing_stm setSizeX:0];
						[existing_stm setSizeY:0];
						
                        [cal_samples replaceObjectAtIndex:i withObject:existing_stm];
						Item_exist = YES;
					}
				}
				
				//If the item's not in the list, add it to the existing list
				if (Item_exist == NO) {
					MWStimulusPlotElement *new_stm = [[MWStimulusPlotElement alloc] initStimElement:stm_type 
                                                                                               Name:stm_name
                                                                                                AtX:cal_sample_H 
                                                                                                AtY:cal_sample_V
                                                                                             WidthX:0 
                                                                                             WidthY:0
                                                                                           Rotation:0];
					[cal_samples addObject:new_stm];
				}
                
                [self triggerUpdate];
			}
		}
	});
}




//=====================================================================================


- (void)reset
{	
    [self setWidth:MAX_ANGLE];
    [self.scaleSlider setFloatValue:MAX_ANGLE];
    
    frameCenterDegrees = NSMakePoint(0, 0);
    
	dispatch_async(serialQueue, ^{
		[eye_samples removeAllObjects];
		[aux_samples removeAllObjects];
        [self triggerUpdate];
	});
}


- (void)setTimeOfTail:(NSTimeInterval)_newTimeOfTail {
	dispatch_async(serialQueue, ^{
		timeOfTail = _newTimeOfTail;
        [self triggerUpdate];
	});
}

/////////////////////////////////////////////////////////////////////////
// Private methods
/////////////////////////////////////////////////////////////////////////


- (void)triggerUpdate {
    if (!updatePending) {
        dispatch_async(dispatch_get_main_queue(), ^{
            if (self.window.visible) {
                // Window is still onscreen.  Trigger an update.
                [self setNeedsDisplay:YES];
            } else {
                // Window is *not* onscreen.  Clear updatePending, so that subsequent calls to
                // triggerUpdate will try again.
                dispatch_sync(serialQueue, ^{
                    updatePending = NO;
                });
            }
        });
        updatePending = YES;
    }
}


@end



























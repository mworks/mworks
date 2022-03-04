/* MWPlotView 

This object is called upon to update the eye window display. It maintains 
two NSMutableArrays. One called eye_samples (originally created by Dave),
which keeps a finite number of past eye sample data to draw eye traces. 
The other NSMutableArray is stm_samples (added by Nuo), which stores any 
drawable objects detected by the stimulus announce event from the datastream and 
use it to draw stimulus on the eye window.

Created by Dave Cox

Modified by Nuo Li


Copy right 2006 MIT. All rights reserved.

*/

#import <Cocoa/Cocoa.h>
#import "MWorksCocoa/MWCocoaEvent.h"
#import "MWorksCore/GenericData.h"

#import "MWTimePlotView.h"

@interface MWPlotView : NSView
{
	float width;
	float gridStepX;
	float gridStepY;
	bool cartesianGrid;
    
    NSRect displayBounds;
    
	NSMutableArray *eye_samples;
	NSMutableArray *aux_samples;
	NSMutableArray *stm_samples;
	NSMutableArray *cal_samples;
	
    MWCocoaEvent *currentEyeH;
    MWCocoaEvent *currentEyeV;

	MWorksTime last_state_change_time;
	int current_state;
	
	NSTimeInterval timeOfTail;	
}

@property(nonatomic, weak) IBOutlet NSSlider *scaleSlider;
@property(nonatomic, weak) IBOutlet NSTextField *scaleLabel;
@property(nonatomic, weak) IBOutlet MWTimePlotView *timePlot;

- (void)setDisplayBounds:(NSRect)bounds;
- (void)setWidth:(float)width;
- (void)addEyeHEvent:(MWCocoaEvent *)event;
- (void)addEyeVEvent:(MWCocoaEvent *)event;
- (void)addAuxHEvent:(MWCocoaEvent *)event;
- (void)addAuxVEvent:(MWCocoaEvent *)event;
- (void)addEyeStateEvent:(MWCocoaEvent *)event;
- (void)acceptStmAnnounce:(mw::Datum *)stm_announce 
					 Time:(MWorksTime)event_time;
- (void)setTimeOfTail:(NSTimeInterval)_newTimeOfTail;
- (void)acceptCalAnnounce:(mw::Datum *)cal_announce;
- (void)reset;

@end




























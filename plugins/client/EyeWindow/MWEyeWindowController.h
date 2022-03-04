/* MWEyeWindowController 

This object reads MWDataEvents from the stream. It watches for certain 
MWDataEvents (e.g. eye sample event, stimulus announce event). 
Once an appropriate event is recieved, it calls on MWPlotView object to update
the eye window display

Created by Dave Cox

Modified by Nuo Li


Copy right 2006 MIT. All rights reserved.

*/

#import "MWorksCore/GenericData.h"
#import "MWEyeWindowOptionController.h"
#import "MWTimePlotView.h"
#import "MWorksCocoa/MWClientProtocol.h"

extern NSString  * MWEyeWindowVariableUpdateNotification;

@class MWPlotView;

@interface MWEyeWindowController : NSWindowController <MWClientPluginWorkspaceState> {
    IBOutlet MWPlotView *plotView;
    IBOutlet MWTimePlotView *timePlotView;
	IBOutlet NSSlider *scaleSlider;
	IBOutlet id<MWClientProtocol> __unsafe_unretained delegate;

	IBOutlet MWEyeWindowOptionController *options;
}

@property (nonatomic, unsafe_unretained, readwrite) id delegate;

- (IBAction)acceptWidth:(id)sender;
- (IBAction)reset:(id)sender;


@end

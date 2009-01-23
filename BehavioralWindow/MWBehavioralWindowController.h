/* MWBehavioralWindowController

This window monitors the behavioral performance of the monkey.
It watches for three variables (which the user put in the text field) in
the event stream, and displays the percentage performance of the Monkey.

Created by Nuo Li  on 9/14/06


Copy right 2006 MIT. All rights reserved.

*/



#import "MonkeyWorksCocoa/MWWindowController.h"
#import "MonkeyWorksCore/GenericData.h"
#import "Narrative/Narrative.h"


@protocol MWDataEventListenerProtocol;

@interface MWBehavioralWindowController : NSWindowController {
		
		// editable text field	(this contains the variable to screen in the data stream)							
		IBOutlet NSTextField *CorrectVariableField;
		IBOutlet NSTextField *FailureVariableField;
		IBOutlet NSTextField *IgnoredVariableField;
		
		// non-editable text field								
		IBOutlet NSTextField *NumberOfCorrectField;
		IBOutlet NSTextField *NumberOfFailureField;
		IBOutlet NSTextField *NumberOfIgnoredField;
		IBOutlet NSTextField *NumberOfTotalField;
		
		IBOutlet NSTextField *PercentCorrectField;
		IBOutlet NSTextField *PercentFailureField;
		IBOutlet NSTextField *PercentIgnoredField;
		
		IBOutlet NSWindow	*optionsSheet;
		
		
		IBOutlet NRTXYChart *chart;
		NRTScatterPlot *percentCorrectPlot;
		
		// codec number for the variables
		int CorrectCodecCode;
		int FailureCodecCode;
		int IgnoredCodecCode;
		
		
		// class variables
		int	numberOfCorrectTrials;
		int	numberOfFailureTrials;
		int	numberOfIgnoredTrials;
		int	numberOfTrials;
		
		int percentCorrect;
		int percentFailure;
		int percentIgnored;
		
		NSMutableArray *percentCorrectHistory;
		NSMutableArray *percentFailureHistory;
		NSMutableArray *percentIgnoredHistory;
		NSMutableArray *totalHistory;
		int maxHistory;
		
		int numberOfCorrectTrialsInSession;
		int numberOfFailureTrialsInSession;
		int numberOfIgnoredTrialsInSession;
		int numberOfTrialsInSession;
		
		int percentCorrectInSession;
		int percentFailureInSession;
		int percentIgnoredInSession;
		
		BOOL VariableCheck;
		
		IBOutlet id delegate;
        IBOutlet NSTextField *addEntryField;
        IBOutlet NSTextView *notebookField;
}


@property int numberOfCorrectTrials;
@property int numberOfFailureTrials;
@property int numberOfIgnoredTrials;
@property int numberOfTrials;

@property int percentCorrect;
@property int percentFailure;
@property int percentIgnored;

@property int numberOfCorrectTrialsInSession;
@property int numberOfFailureTrialsInSession;
@property int numberOfIgnoredTrialsInSession;
@property int numberOfTrialsInSession;

@property int percentCorrectInSession;
@property int percentFailureInSession;
@property int percentIgnoredInSession;


- (void)setDelegate:(id)new_delegate;

- (IBAction)resetPerformance:(id)sender;
- (IBAction)resetPerformanceInSession:(id)sender;
- (IBAction)launchOptionsSheet:(id)sender;
- (IBAction)acceptOptionsSheet:(id)sender;
- (IBAction)cancelOptionsSheet:(id)sender;



- (void)updatePlot;

// Plot data source methods
-(unsigned)numberOfDataClustersForPlot:(NRTPlot *)plot;
-(NSDictionary *)clusterCoordinatesForPlot:(id)plot andDataClusterIndex:(unsigned)entryIndex;


// Notebook methods
-(IBAction) addEntry:(id)sender;
-(void) addEntryString:(NSString *)str;
-(IBAction) addBehaviorSummaryEntry:(id)sender;

@end
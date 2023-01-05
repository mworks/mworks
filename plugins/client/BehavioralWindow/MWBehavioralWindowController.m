/* MWBehavioralWindowController 

Created by Nuo Li  on 9/14/06

Copy right 2006 MIT. All rights reserved.

*/


#import "MWorksCore/Utilities.h"
#import "MWorksCocoa/MWCocoaEvent.h"
#import "MWorksCocoa/MWClientServerBase.h"
#import "MWBehavioralWindowController.h"

#define BEHAVIORIAL_CALLBACK_KEY "MWorksBehaviorialWindowCallbackKey"

#define BEHAVIORIAL_WINDOW_SUCCESS_KEY @"Behavioral Window - success"
#define BEHAVIORIAL_WINDOW_FAILURE_KEY @"Behavioral Window - failure"
#define BEHAVIORIAL_WINDOW_IGNORED_KEY @"Behavioral Window - ignored"

NSString *percentCorrectPlotIdentifier = @"PercentCorrectLinePlot";


@implementation MWBehavioralWindowController

@synthesize numberOfCorrectTrials;
@synthesize numberOfFailureTrials;
@synthesize numberOfIgnoredTrials;
@synthesize numberOfTrials;

@synthesize percentCorrect;
@synthesize percentFailure;
@synthesize percentIgnored;

@synthesize numberOfCorrectTrialsInSession;
@synthesize numberOfFailureTrialsInSession;
@synthesize numberOfIgnoredTrialsInSession;
@synthesize numberOfTrialsInSession;

@synthesize percentCorrectInSession;
@synthesize percentFailureInSession;
@synthesize percentIgnoredInSession;

- (void)setupChart {
	// Read chart attributes from plist file.
    //NSDictionary *attribsDict = [NSDictionary dictionaryWithContentsOfFile:
    //    [[NSBundle mainBundle] pathForResource:@"MWBehavioralHistoryChartAttributes" ofType:@"plist"]];
    //[chart setAttributesFromDictionary:attribsDict];
    
    // Set the plot area size and position, which can be done via the configuration
    // file, but is probably best determined dynamically from the NRTXYChart view's
    // bounds.
    // Also set the position of the chart title. 
    // Note that all of these things can also be set via the configuration file.
    NSRect b = [chart bounds];
    [chart setAttributesFromDictionary:
        [NSDictionary dictionaryWithObjectsAndKeys:
            @"",
            NRTTitleCA,
         
            [NSArray arrayWithObjects:
                [NSNumber numberWithFloat:b.origin.x + 0], 
                [NSNumber numberWithFloat:b.origin.x + b.size.width], nil],
            NRTPlotAreaFrameXRangeCA,
            
            [NSArray arrayWithObjects:
                [NSNumber numberWithFloat:b.origin.y + 0], 
                [NSNumber numberWithFloat:b.origin.y + b.size.height], nil],
            NRTPlotAreaFrameYRangeCA, 
            
			
			[NSArray arrayWithObjects:
                [NSNumber numberWithFloat:0], 
                [NSNumber numberWithFloat:103], nil],
            NRTLeftYAxisCoordinateRangeCA, 
			
			[NSArray arrayWithObjects:
                [NSNumber numberWithFloat:0], 
                [NSNumber numberWithFloat:maxHistory], nil],
            NRTBottomXAxisCoordinateRangeCA, 
			
			// Toned down bottom axis ticks
			[NSNumber numberWithFloat:1], 
            NRTBottomXAxisMajorTickWidthCA,
			
			[NSNumber numberWithFloat:3], 
            NRTBottomXAxisMajorTickLengthCA,			
			
			[NSNumber numberWithFloat:0], 
            NRTBottomXAxisMinorTickWidthCA,
			
			[NSNumber numberWithFloat:0], 
            NRTBottomXAxisMinorTickLengthCA,			
			
			// No top axis ticks
			[NSNumber numberWithFloat:0], 
            NRTTopXAxisMajorTickWidthCA,
			
			[NSNumber numberWithFloat:0], 
            NRTTopXAxisMajorTickLengthCA,			
			
			[NSNumber numberWithFloat:0], 
            NRTTopXAxisMinorTickWidthCA,
			
			[NSNumber numberWithFloat:0], 
            NRTTopXAxisMinorTickLengthCA,	
			
			
			// Toned down bottom axis ticks
			[NSNumber numberWithFloat:1], 
            NRTLeftYAxisMajorTickWidthCA,
			
			[NSNumber numberWithFloat:3], 
            NRTLeftYAxisMajorTickLengthCA,			
			
			[NSNumber numberWithFloat:0], 
            NRTLeftYAxisMinorTickWidthCA,
			
			[NSNumber numberWithFloat:0], 
            NRTLeftYAxisMinorTickLengthCA,			
			
			// No top axis ticks
			[NSNumber numberWithFloat:0], 
            NRTRightYAxisMajorTickWidthCA,
			
			[NSNumber numberWithFloat:0], 
            NRTRightYAxisMajorTickLengthCA,			
			
			[NSNumber numberWithFloat:0], 
            NRTRightYAxisMinorTickWidthCA,
			
			[NSNumber numberWithFloat:0], 
            NRTRightYAxisMinorTickLengthCA,
			
			
			// Toned down axes
			[NSNumber numberWithFloat:1],
			NRTBottomXAxisWidthCA,
			
			[NSNumber numberWithFloat:1],
			NRTLeftYAxisWidthCA,
			
			[NSNumber numberWithFloat:0],
			NRTTopXAxisWidthCA,
			
			[NSNumber numberWithFloat:0],
			NRTRightYAxisWidthCA,
			
			//[NSArray arrayWithObjects:
                //[NSNumber numberWithFloat:0], 
				//[NSNumber numberWithFloat:50],
				//[NSNumber numberWithFloat:100],
				//[NSNumber numberWithFloat:150],
                //[NSNumber numberWithFloat:maxHistory],
				//nil],
            [NSArray array],
			NRTBottomXAxisTickCoordinatesCA,
			
			//[NSArray arrayWithObjects:
				// [NSNumber numberWithFloat:0], 
				// [NSNumber numberWithFloat:50],
				//[NSNumber numberWithFloat:100], 
				//nil],
            [NSArray array],
			NRTLeftYAxisTickCoordinatesCA,
			
			
			[NSArray arrayWithObjects:
                [NSNumber numberWithFloat:0.0], 
                [NSNumber numberWithFloat:0],
				[NSNumber numberWithFloat:0],
                [NSNumber numberWithFloat:1.0], nil],
			NRTPlotAreaBackgroundRGBACA,
			
			[NSArray arrayWithObjects:
                [NSNumber numberWithFloat:0], 
                [NSNumber numberWithFloat:0],
				[NSNumber numberWithFloat:0],
                [NSNumber numberWithFloat:0.0], nil],
			NRTChartBackgroundRGBACA,
			
			/* [NSArray arrayWithObjects:
			[NSNumber numberWithFloat:b.origin.x + b.size.width * 0.5],
			[NSNumber numberWithFloat:b.origin.y + b.size.height - 25], nil],
		NRTTitlePlotViewVectorCA,*/
            
            nil]];
	
    // Add plots
    //[self addLinePlot];
	// [self addHistogramPlot];
	
	// Set plot attributes, which are defined in the NRTScatterPlot ancestor class NRTPlotObject
    percentCorrectPlot = [[NRTScatterPlot alloc] 
									initWithIdentifier:percentCorrectPlotIdentifier 
										 andDataSource:self];
    [percentCorrectPlot setAttribute:[NSNumber numberWithFloat:0.5] forKey:NRTLineWidthAttrib];
    [percentCorrectPlot setAttribute:[NSColor colorWithCalibratedRed:0.1 green:0.7 blue:0.4 alpha:1.0] 
							  forKey:NRTLineColorAttrib];
    [percentCorrectPlot setConnectPoints:YES];
	
	// Create and set the cluster graphic of the plot. Here we will just create a filled
    // symbol. We set the stroke and fill colors of the symbol.
    NRTSymbolClusterGraphic *clusterGraphic = [[NRTSymbolClusterGraphic alloc] init];
    [clusterGraphic setAttribute:[NSColor colorWithCalibratedRed:0.0 green:0.0 blue:1.0 alpha:1.0] 
						  forKey:NRTFillColorAttrib];
    [clusterGraphic setAttribute:[NSColor colorWithCalibratedRed:0.75 green:0.75 blue:1.0 alpha:1.0] 
						  forKey:NRTLineColorAttrib];
    [clusterGraphic setAttribute:[NSNumber numberWithFloat:0.7] forKey:NRTLineWidthAttrib];
	
    // Now we need to actually set the form of the symbol, which will be a circle with a line.
    // It can be anything you can draw with NSBezierPath. It could be a square, a triangle, or
    // a giraffe. 
    NSBezierPath *symbolPath = [NSBezierPath bezierPath];
    [symbolPath moveToPoint:NSMakePoint(0.0, 0.0)];
    [symbolPath appendBezierPathWithOvalInRect:NSMakeRect( -1.0, -1.0, 1.0, 1.0 )];
    [clusterGraphic setBezierPath:symbolPath];
    [clusterGraphic setSize:NSMakeSize(1.0, 1.0)];
    
    [percentCorrectPlot setClusterGraphic:clusterGraphic];
    
	[chart setUseImageCache:NO];
	
    // Finally add the plot to chart
    [chart addPlot:percentCorrectPlot];
	
	[chart layout];
	
}

- (void)awakeFromNib {
    // Register default values for preferences
    {
        NSMutableDictionary *defaultValues = [NSMutableDictionary dictionary];
        
        [defaultValues setObject:@"success" forKey:BEHAVIORIAL_WINDOW_SUCCESS_KEY];
        [defaultValues setObject:@"failure" forKey:BEHAVIORIAL_WINDOW_FAILURE_KEY];
        [defaultValues setObject:@"ignore" forKey:BEHAVIORIAL_WINDOW_IGNORED_KEY];
        
        [[NSUserDefaults standardUserDefaults] registerDefaults:defaultValues];
    }
    
	CorrectCodecCode = -1;
	FailureCodecCode = -1;
	IgnoredCodecCode = -1;
	
	numberOfCorrectTrials = 0;
	numberOfFailureTrials = 0;
	numberOfIgnoredTrials = 0;
	numberOfTrials = 0;
	
	percentCorrect = 0;
	percentFailure = 0;
	percentIgnored = 0;
	
	numberOfCorrectTrialsInSession = 0;
	numberOfFailureTrialsInSession = 0;
	numberOfIgnoredTrialsInSession = 0;
	numberOfTrialsInSession = 0;
	
	percentCorrectInSession = 0;
	percentFailureInSession = 0;
	percentIgnoredInSession = 0;
	
	[PercentCorrectField setDrawsBackground:NO];
	[PercentFailureField setDrawsBackground:NO];
	[PercentIgnoredField setDrawsBackground:NO];
	
	
	percentCorrectHistory = [[NSMutableArray alloc] init];
	percentFailureHistory = [[NSMutableArray alloc] init];
	percentIgnoredHistory = [[NSMutableArray alloc] init];
	totalHistory = [[NSMutableArray alloc] init];
	maxHistory = 100;
	
	
	[self setupChart];
	
}

- (void)setDelegate:(id)new_delegate {
	if(![new_delegate respondsToSelector:@selector(registerEventCallbackWithReceiver:
												   selector:
												   callbackKey:
                                                   onMainThread:)] ||
	   ![new_delegate respondsToSelector:@selector(unregisterCallbacksWithKey:)] ||
	   ![new_delegate respondsToSelector:@selector(registerEventCallbackWithReceiver:
												   selector:
												   callbackKey:
												   forVariableCode:
                                                   onMainThread:)] ||
	   ![new_delegate respondsToSelector:@selector(codeForTag:)]) {
		[NSException raise:NSInternalInconsistencyException
					format:@"Delegate doesn't respond to required methods for MWBehavioralWindowController"];		
	}
	
	delegate = new_delegate;
	[delegate registerEventCallbackWithReceiver:self 
                                       selector:@selector(receiveCodec:)
                                    callbackKey:BEHAVIORIAL_CALLBACK_KEY
                                forVariableCode:RESERVED_CODEC_CODE
                                   onMainThread:YES];
}


// if a correct trial variable is received
- (void)serviceCorrectEvent:(MWCocoaEvent *)event {		
		
    self.numberOfCorrectTrials += 1;
    self.numberOfCorrectTrialsInSession += 1;
    self.numberOfTrials += 1;
    self.numberOfTrialsInSession += 1;
                            
    [PercentCorrectField setDrawsBackground:YES];
    [PercentFailureField setDrawsBackground:NO];
    [PercentIgnoredField setDrawsBackground:NO];
    
    [self updatePercentages];
}

- (void)serviceFailureEvent:(MWCocoaEvent *)event {
		
    self.numberOfFailureTrials += 1;
    self.numberOfFailureTrialsInSession += 1;
    self.numberOfTrials += 1;
    self.numberOfTrialsInSession += 1;
    
    [PercentCorrectField setDrawsBackground:NO];
    [PercentFailureField setDrawsBackground:YES];
    [PercentIgnoredField setDrawsBackground:NO];
    //[PercentFailureField setBackgroundColor:[NSColor redColor]];

    [self updatePercentages];
}


- (void)serviceIgnoredEvent:(MWCocoaEvent *)event {
		
    self.numberOfIgnoredTrials += 1;
    self.numberOfIgnoredTrialsInSession += 1;
    self.numberOfTrials += 1;
    self.numberOfTrialsInSession += 1;
    
    [PercentCorrectField setDrawsBackground:NO];
    [PercentFailureField setDrawsBackground:NO];
    [PercentIgnoredField setDrawsBackground:YES];
    //[PercentIgnoredField setBackgroundColor:[NSColor redColor]];
    
    [self updatePercentages];
}
	
- (void) updatePercentages {		
    self.percentCorrect = (int)((double)numberOfCorrectTrials/(double)numberOfTrials*100);
    self.percentCorrectInSession = (int)((double)numberOfCorrectTrialsInSession/(double)numberOfTrialsInSession*100);
    
    self.percentFailure = (int)((double)numberOfFailureTrials/(double)numberOfTrials*100);
    self.percentFailureInSession = (int)((double)numberOfFailureTrialsInSession/(double)numberOfTrialsInSession*100);
    
    self.percentIgnored = (int)((double)numberOfIgnoredTrials/(double)numberOfTrials*100);
    self.percentIgnoredInSession = (int)((double)numberOfIgnoredTrialsInSession/(double)numberOfTrialsInSession*100);
    
    [percentCorrectHistory addObject:[NSNumber numberWithDouble:percentCorrect]];
    [percentFailureHistory addObject:[NSNumber numberWithDouble:percentFailure]];
    [percentIgnoredHistory addObject:[NSNumber numberWithDouble:percentIgnored]];
    [totalHistory addObject:[NSNumber numberWithInt:numberOfTrials]];
    
    
    if([totalHistory count] > maxHistory){
        [percentCorrectHistory removeObjectAtIndex:0];
        [percentFailureHistory removeObjectAtIndex:0];
        [percentIgnoredHistory removeObjectAtIndex:0];
        [totalHistory removeObjectAtIndex:0];
    }
    
    [self updatePlot];
    
    [self performSelectorOnMainThread: @selector(setNeedsDisplayOnMainThread:) 
                           withObject: chart waitUntilDone: NO];	
    

	
	
}


- (void)setNeedsDisplayOnMainThread:(id)obj
{
	[obj setNeedsDisplay:YES];
	//NSLog(@"redisplaying");
}

- (void)updatePlot
{
	//@synchronized(chart){
		[chart layout];
	//}
}


- (IBAction)launchOptionsSheet:(id)sender{
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
	[NSApp beginSheet:optionsSheet modalForWindow:[NSApp mainWindow]
		modalDelegate:nil didEndSelector:nil contextInfo:nil];
#pragma clang diagnostic pop
}


- (IBAction)acceptOptionsSheet:(id)sender{
    // Finish editing in all fields
    if (![optionsSheet makeFirstResponder:optionsSheet]) {
        [optionsSheet endEditingFor:nil];
    }
    
    [optionsSheet orderOut:self];
    [NSApp endSheet:optionsSheet];
    [self updateCodes];
}


- (IBAction)resetPerformance:(id)sender {
	
        [self addBehaviorSummaryEntry:self];
        [self addNotebookEntryString:@"<reset>"];
    
		// reset button clears the past performance and start over the counting
		
		self.numberOfCorrectTrials = 0;
		self.numberOfFailureTrials = 0;
		self.numberOfIgnoredTrials = 0;
		self.numberOfTrials = 0;
		
		self.percentCorrect = 0;
		self.percentFailure = 0;
		self.percentIgnored = 0;
		
		[PercentCorrectField setDrawsBackground:NO];
		[PercentFailureField setDrawsBackground:NO];
		[PercentIgnoredField setDrawsBackground:NO];
		
		[percentCorrectHistory removeAllObjects];
		[percentFailureHistory removeAllObjects];
		[percentIgnoredHistory removeAllObjects];
		[totalHistory removeAllObjects];
		
		[self updatePlot];

}


- (IBAction)resetPerformanceInSession:(id)sender {
	
    [self addBehaviorSummaryEntry:self];
	[self addNotebookEntryString:@"<reset session>"];
		
		self.numberOfCorrectTrials = 0;
		self.numberOfFailureTrials = 0;
		self.numberOfIgnoredTrials = 0;
		self.numberOfTrials = 0;
		
		self.percentCorrect = 0;
		self.percentFailure = 0;
		self.percentIgnored = 0;
		
		self.numberOfCorrectTrialsInSession = 0;
		self.numberOfFailureTrialsInSession = 0;
		self.numberOfIgnoredTrialsInSession = 0;
		self.numberOfTrialsInSession = 0;
		
		self.percentCorrectInSession = 0;
		self.percentFailureInSession = 0;
		self.percentIgnoredInSession = 0;
		
		[PercentCorrectField setDrawsBackground:NO];
		[PercentFailureField setDrawsBackground:NO];
		[PercentIgnoredField setDrawsBackground:NO];
		
		[percentCorrectHistory removeAllObjects];
		[percentFailureHistory removeAllObjects];
		[percentIgnoredHistory removeAllObjects];
		[totalHistory removeAllObjects];
		
		[self updatePlot];
	
}


/*******************************************************************
*                           Private Methods
*******************************************************************/
- (void)receiveCodec: (MWCocoaEvent *)event {
    [self updateCodes];
}


// This methods find the codec number for the variables that keep track of good trials
//versus bad trials, it does so by matching the tag names to the variables in the experiment.
- (void)updateCodes {
    if (!delegate) {
        return;
    }
    
	[delegate unregisterCallbacksWithKey:BEHAVIORIAL_CALLBACK_KEY];
	
    CorrectCodecCode = [[delegate codeForTag:[CorrectVariableField stringValue]] intValue];
    FailureCodecCode = [[delegate codeForTag:[FailureVariableField stringValue]] intValue];
    IgnoredCodecCode = [[delegate codeForTag:[IgnoredVariableField stringValue]] intValue];
	
	// if variables entered by the user was not found in the experiment, issue a warning in the console.
	if (CorrectCodecCode == -1) {
		mwarning(M_NETWORK_MESSAGE_DOMAIN, "Variable for success trials: %s was not found.",[[CorrectVariableField stringValue] cStringUsingEncoding:NSASCIIStringEncoding]);
	} else {
		[delegate registerEventCallbackWithReceiver:self 
                                           selector:@selector(serviceCorrectEvent:)
                                        callbackKey:BEHAVIORIAL_CALLBACK_KEY
                                    forVariableCode:CorrectCodecCode
                                       onMainThread:YES];		
	}
	
	if (FailureCodecCode == -1) {
		mwarning(M_NETWORK_MESSAGE_DOMAIN, "Variable for failure trials: %s was not found.",[[FailureVariableField stringValue] cStringUsingEncoding:NSASCIIStringEncoding]);
	} else {
		[delegate registerEventCallbackWithReceiver:self 
                                           selector:@selector(serviceFailureEvent:)
                                        callbackKey:BEHAVIORIAL_CALLBACK_KEY
                                    forVariableCode:FailureCodecCode
                                       onMainThread:YES];				
	}
	
	if (IgnoredCodecCode == -1) {
		mwarning(M_NETWORK_MESSAGE_DOMAIN, "Variable for ignored trials: %s was not found.",[[IgnoredVariableField stringValue] cStringUsingEncoding:NSASCIIStringEncoding]);
	} else {
		[delegate registerEventCallbackWithReceiver:self 
                                           selector:@selector(serviceIgnoredEvent:)
                                        callbackKey:BEHAVIORIAL_CALLBACK_KEY
                                    forVariableCode:IgnoredCodecCode
                                       onMainThread:YES];				
	}
	
    // re-register for the codec
	[delegate registerEventCallbackWithReceiver:self 
                                       selector:@selector(receiveCodec:)
                                    callbackKey:BEHAVIORIAL_CALLBACK_KEY
                                forVariableCode:RESERVED_CODEC_CODE
                                   onMainThread:YES];

}


// Plot data source methods
-(unsigned)numberOfDataClustersForPlot:(NRTPlot *)plot
{
	int num = 0;
	
	if ( [[plot identifier] isEqual:percentCorrectPlotIdentifier] ) {
		num =  [percentCorrectHistory count];
		//NSLog(@"N Data Clusters: %d", num);
    }
	
	return num;
}


-(NSDictionary *)clusterCoordinatesForPlot:(id)plot andDataClusterIndex:(unsigned)entryIndex
{
    float x, y;
	
	x = (float)entryIndex;
	y = 0.0;
	
	if([[plot identifier] isEqual:percentCorrectPlotIdentifier]){
		y = [[percentCorrectHistory objectAtIndex:entryIndex] floatValue];
	}
	
    //NSLog(@"x,y: %g, %g", x, y);
	
    return [NSDictionary dictionaryWithObjectsAndKeys:
        [NSNumber numberWithFloat:x],	NRTXClusterIdentifier,
        [NSNumber numberWithFloat:y], 	NRTYClusterIdentifier, nil];
	
	
}

-(void) addNotebookEntryString:(NSString *)str {
    [[delegate notebook] addEntry:str];
    [notebookField setString:[[delegate notebook] content]];
    [notebookField scrollRangeToVisible:NSMakeRange([[notebookField textStorage] length],0)];
}

-(IBAction) addNotebookEntry:(id)sender {
    [self addNotebookEntryString:[addNotebookEntryField stringValue]];
}

-(IBAction) addBehaviorSummaryEntry:(id)sender{
    NSString *format_string;
    format_string = @"<%d trials (since last reset; %d total): %d %% correct / %d%% failure / %d%% ignored>";
    NSString *summary_string = [NSString stringWithFormat:format_string, 
                                                        numberOfTrials, 
                                                        numberOfTrialsInSession, 
                                                        percentCorrect, 
                                                        percentFailure, 
                                                        percentIgnored, Nil];
    [self addNotebookEntryString:summary_string];
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-implementations"
- (void)controlTextDidEndEditing:(NSNotification *)aNotification {
    [self addNotebookEntry:self];
    [addNotebookEntryField setStringValue:@""];
}
#pragma clang diagnostic pop


- (NSDictionary *)workspaceState {
    NSMutableDictionary *workspaceState = [NSMutableDictionary dictionary];
    NSUserDefaults *ud = [NSUserDefaults standardUserDefaults];
    
    [workspaceState setObject:[ud stringForKey:BEHAVIORIAL_WINDOW_SUCCESS_KEY] forKey:@"success"];
    [workspaceState setObject:[ud stringForKey:BEHAVIORIAL_WINDOW_FAILURE_KEY] forKey:@"failure"];
    [workspaceState setObject:[ud stringForKey:BEHAVIORIAL_WINDOW_IGNORED_KEY] forKey:@"ignored"];
    
    return workspaceState;
}


- (void)setWorkspaceState:(NSDictionary *)workspaceState {
    NSUserDefaults *ud = [NSUserDefaults standardUserDefaults];
    
    NSString *newSuccess = [workspaceState objectForKey:@"success"];
    if (newSuccess && [newSuccess isKindOfClass:[NSString class]]) {
        [ud setObject:newSuccess forKey:BEHAVIORIAL_WINDOW_SUCCESS_KEY];
    }
    
    NSString *newFailure = [workspaceState objectForKey:@"failure"];
    if (newFailure && [newFailure isKindOfClass:[NSString class]]) {
        [ud setObject:newFailure forKey:BEHAVIORIAL_WINDOW_FAILURE_KEY];
    }
    
    NSString *newIgnored = [workspaceState objectForKey:@"ignored"];
    if (newIgnored && [newIgnored isKindOfClass:[NSString class]]) {
        [ud setObject:newIgnored forKey:BEHAVIORIAL_WINDOW_IGNORED_KEY];
    }
    
    [self updateCodes];
}


@end





























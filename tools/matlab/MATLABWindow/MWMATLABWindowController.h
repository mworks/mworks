/* MWMATLABWindowController */
// COMMENTS!!!!!

#import "MWorksCocoa/MWClientProtocol.h"
#import "MATLABInterface.h"
#import "MWVariableList.h"

@protocol MWDataEventListenerProtocol;

@interface MWMATLABWindowController : NSWindowController <MWClientPluginWorkspaceState> {

	IBOutlet MATLABInterface *mi;
	IBOutlet id<MWClientProtocol> __unsafe_unretained delegate;
    IBOutlet NSTextView *logTextView;
    IBOutlet NSButton *scrollToBottomButton;
	
	NSMutableArray *eventList;
	NSMutableArray *executionList;
	
	NSString *sync_event_name;
	int running;
	int processing;
	NSString *number_to_process_string;
	NSString *matlab_file_name;
	NSArray *default_selected_variables;
	NSString *logTextContent;
	
	BOOL collectingEvents;
	BOOL scrollToBottom; // todo: this should really be implemented with accessors
	
	NSRecursiveLock *matlabLock;
	IBOutlet MWVariableList *vl;
	Datum *savedCodec;
}

@property (nonatomic, readwrite, unsafe_unretained) id delegate;
@property (readwrite, copy) NSString *syncEventName;
@property (readwrite, copy) NSString *numberToProcessString;
@property (readwrite, copy) NSString *matlabFileName;
@property (nonatomic, readwrite, copy) NSString *logTextContent;

- (IBAction)chooseMATLABFile:(id)sender;
- (IBAction)primeMATLABEngine:(id)sender;
- (IBAction)resetAction:(id)sender;
- (IBAction)changeScrollToBottom:(id)sender;

- (int)processing;
- (void)setProcessing:(int)new_processing;
- (int)running;
- (void)setRunning:(int)new_running;
- (void)appendLogText:(NSString *)logText;
- (void)doScrollToBottom;
// private, do not call these (they are called in main thread to avoid UI race problems)
- (void)appendLogTextMain:(NSString *)logText;
- (void)doScrollToBottomMain;
						   
@end


@interface MWMATLABWindowController(Delegate)
- (void)updateVariableFilter;
@end

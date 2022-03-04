#import "MATLABInterface.h"
#import "MWorksCocoa/MWCocoaEvent.h"
#import "MWorksCore/GenericData.h"
#import "MWorksCore/Event.h"
#import "MWorksCore/Utilities.h"
#import "MWorksCore/ScarabServices.h"
#import "engine.h"
#import "matrix.h"
#import "mat.h"
#import "MWorksMATLABTools.h"
#import "MWMATLABWindowController.h"


#define OUTPUT_BUFFER_SIZE 32768

#define STREAM @"MWorks Stream"

#define ml_ADD_MATLAB_PATH @"addpath('/Library/Application Support/MWorks/Scripting/MATLAB')"

#define ml_FILENAME "filename"
#define ml_EVENT_CODEC "event_codec"
#define ml_EVENTS "events"

#define MATLAB_APP_PATH @"/Applications/MATLAB/bin/matlab"

#ifdef __x86_64__
#  define MATLAB_ARCH "maci64"
#else
#  define MATLAB_ARCH "maci"
#endif

#define MATLAB_ARGS "-nosplash -" MATLAB_ARCH

#define ml_RETVAL @"retval"

@interface MATLABInterface (PrivateMethods)
- (Engine *)getMATLABEngine;
- (mxArray *)createTopLevelDataStructure:(NSString *)name;
@end

@implementation MATLABInterface

- (id) init {
	self = [super init];
	if (self != nil) {
		retval = NULL;
        outputBuffer = [[NSMutableData alloc] initWithLength:OUTPUT_BUFFER_SIZE];
		eventStructsQueue = [[NSMutableArray alloc] init];
		interfaceLock = [[NSLock alloc] init];
	}
	return self;
}

- (void) dealloc {

	if(retval) {
		mxDestroyArray(retval);
	}
	
}

- (id)delegate {
    return delegate;
}

- (void)setDelegate:(id)newDelegate {
    delegate = newDelegate;
}


- (void)setMatlabFile:(NSString *)file {
	[interfaceLock lock];
	matlabFile = [file copy];
	[interfaceLock unlock];
}

- (NSString *)matlabFile {
	[interfaceLock lock];
	NSString *retstring = [matlabFile copy];
	[interfaceLock unlock];
	return retstring;
}	

- (void)resetRetval {
	[interfaceLock lock];
	if(retval) {
        [self runCleanupFile];
		mxDestroyArray(retval);
		retval = 0;
	}
	[interfaceLock unlock];
}

- (void)logMATLABOutput {
	NSString * tStr = [NSString stringWithUTF8String:(char *)[outputBuffer mutableBytes]];
    [delegate appendLogText:tStr];
    [outputBuffer resetBytesInRange:NSMakeRange(0, OUTPUT_BUFFER_SIZE)];
}	

- (mxArray *)createDataStruct:(NSArray *)dataEventList
					withCodec:(Datum *)codec {
	
	[interfaceLock lock];
	mxArray *codecStruct = getCodec(datumToScarabDatum(*codec).get());
	int nevents = [dataEventList count];
	
	mxArray *data_struct = [self createTopLevelDataStructure:STREAM];
	if(codecStruct) {
		mxSetField(data_struct, 0, 
				   ml_EVENT_CODEC, 
				   codecStruct);
	} else {
		// no codec
		merror(M_CLIENT_MESSAGE_DOMAIN, "Illegal codec in MATLAB window");
		[interfaceLock unlock];
		return 0;
	}
	
	mxArray *events = createTopLevelEventStruct(nevents);
	
	mxArray *old_events = mxGetField(data_struct, 0, ml_EVENTS);
	if(old_events) {
		mxDestroyArray(old_events);
	}
	mxSetField(data_struct, 0, ml_EVENTS, events);
	
	int nread = 0;
	NSEnumerator *enumerator = [dataEventList objectEnumerator]; 
	MWCocoaEvent *event;
	while( (event = [enumerator nextObject]) ) { 
		Datum data(*[event data]);
		
		Event de([event code], 
				  [event time], 
				  data);
		
		auto datum = eventToScarabEventDatum(de);
		
		// All events should be scarab lists
		if (datum.get()->type != SCARAB_LIST) {
			break;
		}
		
		// Convert and add to event list
		insertDatumIntoEventList(events, nread, datum.get());
		
		nread++;
	}
	
	[interfaceLock unlock];
	return data_struct;
}


- (void)runMatlabFile:(mxArray *)data_struct {	
	[interfaceLock lock];
	NSString *matlabFunction = [[matlabFile lastPathComponent] stringByDeletingPathExtension];

	Engine *e = [self getMATLABEngine];
	
	NSString *addpath_command = [NSString stringWithFormat:@"addpath('%@')", [matlabFile stringByDeletingLastPathComponent]];
	engEvalString(e, [addpath_command cStringUsingEncoding:NSASCIIStringEncoding]);	
	[self logMATLABOutput];
	
	NSString *cmd;	
	if(retval) {
		engPutVariable(e, 
					   [ml_RETVAL cStringUsingEncoding:NSASCIIStringEncoding], 
					   retval);
		cmd = [NSString stringWithFormat:@"%@=%@(%s,%@); ", ml_RETVAL, matlabFunction, ml_EVENTS, ml_RETVAL];
		mxDestroyArray(retval);
	} else {
		cmd = [NSString stringWithFormat:@"%@=%@(%s); ", ml_RETVAL, matlabFunction, ml_EVENTS];
	}
	engPutVariable(e, ml_EVENTS, data_struct);

	// make cmd return error output by wrapping in try; catch
	engEvalString(e, "if ~exist('printErrorStack'), disp('printErrorStack.m not found, cannot display error output');end");
	[self logMATLABOutput];
	
	NSString *catchCmd = [NSString stringWithFormat:@"try, %@, catch ex, printErrorStack(ex); end", cmd]; 
	engEvalString(e, [catchCmd cStringUsingEncoding:NSASCIIStringEncoding]);
	[self logMATLABOutput];
	
	retval = engGetVariable(e, 
							[ml_RETVAL cStringUsingEncoding:NSASCIIStringEncoding]);
	[interfaceLock unlock];
}

- (void)runCleanupFile {
    NSString *cleanupFile = [matlabFile.stringByDeletingPathExtension stringByAppendingString:@"_cleanup.m"];
    if (![[[NSFileManager alloc] init] fileExistsAtPath:cleanupFile]) {
        return;
    }
    NSString *cleanupFunction = cleanupFile.lastPathComponent.stringByDeletingPathExtension;
    
    Engine *e = [self getMATLABEngine];
    
    NSString *addpath_command = [NSString stringWithFormat:@"addpath('%@')", [cleanupFile stringByDeletingLastPathComponent]];
    engEvalString(e, [addpath_command cStringUsingEncoding:NSASCIIStringEncoding]);
    [self logMATLABOutput];
    
    engPutVariable(e,
                   [ml_RETVAL cStringUsingEncoding:NSASCIIStringEncoding],
                   retval);
    NSString *cmd = [NSString stringWithFormat:@"%@(%@); ", cleanupFunction, ml_RETVAL];
    
    // make cmd return error output by wrapping in try; catch
    engEvalString(e, "if ~exist('printErrorStack'), disp('printErrorStack.m not found, cannot display error output');end");
    [self logMATLABOutput];
    
    NSString *catchCmd = [NSString stringWithFormat:@"try, %@, catch ex, printErrorStack(ex); end", cmd];
    engEvalString(e, [catchCmd cStringUsingEncoding:NSASCIIStringEncoding]);
    [self logMATLABOutput];
}

- (void)startMATLABEngine {
	[interfaceLock lock];
	[self getMATLABEngine];
	[interfaceLock unlock];
}

////////////////////////////////////////////////////////////////////////////////
// private methods
////////////////////////////////////////////////////////////////////////////////
- (Engine *)getMATLABEngine {
	
	if(!matlabEngine) {
        // Resolve the MATLAB symlink so we don't get "same name as a MATLAB builtin" warnings
        // every time we call addpath
        NSString *matlabAppPath = [MATLAB_APP_PATH stringByResolvingSymlinksInPath];
        NSString *matlabStartupCommand = [NSString stringWithFormat:@"%@ %s", matlabAppPath, MATLAB_ARGS];

        NSLog(@"Launching MATLAB with command \"%@\"", matlabStartupCommand);
		matlabEngine = engOpen([matlabStartupCommand UTF8String]);

		if (!matlabEngine) {
			[delegate appendLogText:@"** engOpen failed in starting Matlab\n"];
			[delegate appendLogText:[NSString stringWithFormat:@"** command used was \"%@\"\n", matlabStartupCommand]];			
		} else {
				
			engSetVisible(matlabEngine, 1);
			engOutputBuffer(matlabEngine, (char *)[outputBuffer mutableBytes], OUTPUT_BUFFER_SIZE);

			engEvalString(matlabEngine, 
						  [ml_ADD_MATLAB_PATH cStringUsingEncoding:NSASCIIStringEncoding]);
		}

	}	

	// check to see if engine is still running
	mxArray *dummyArray = mxCreateDoubleScalar(0);
	if(engPutVariable(matlabEngine, "MW_DUMMY_VAR", dummyArray)) {
		[delegate appendLogText:@"** MATLAB engine found dead"];
		// if it's not, start it up again
		mxDestroyArray(dummyArray);
		engClose(matlabEngine);
		matlabEngine = 0;
		return [self getMATLABEngine];
	}
	mxDestroyArray(dummyArray);
	
	
	return matlabEngine;
}

- (mxArray *)createTopLevelDataStructure:(NSString *)name {
	// *****************************************************************
	// Create the file struct
	// events field will contain the actual events
	// event_types field will include a code that identifies the
	//			   event types (e.g. message, data, etc.)
	// event_codec field will contain the event name / code dictionary
	// *****************************************************************
	
	mwSize ndims = 1;
	mwSize data_dims = 1;
	const char *data_field_names[] = {ml_FILENAME, ml_EVENT_CODEC, ml_EVENTS};
	int data_nfields = 3; // filename, codec, event
	mxArray *dataStruct = mxCreateStructArray(ndims, &data_dims, data_nfields, 
											   data_field_names);
	
	mxArray *old_filename = mxGetField(dataStruct, 0, ml_FILENAME);
	if(old_filename){
		mxDestroyArray(old_filename);
	}
	mxSetField(dataStruct, 
			   0, 
			   ml_FILENAME, 
			   mxCreateString([name cStringUsingEncoding:NSASCIIStringEncoding]));	
	
	return dataStruct;
}

@end

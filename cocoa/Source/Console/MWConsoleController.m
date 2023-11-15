//
//  MWConsoleController.m
//  MWorksGUIComponents
//
//  Created by Ben Kennedy on 7/23/07.
//  Copyright 2007 MIT. All rights reserved.
//

#import "MWMessageContainer.h"
#import "MWConsoleController.h"
#import "MWCocoaEvent.h"
#import "MWClientServerBase.h"

#import <UniformTypeIdentifiers/UniformTypeIdentifiers.h>

#define MW_CONSOLE_CONTROLLER_CALLBACK_KEY "MWorksCocoa console controller callback key"
#define MW_CONSOLE_MAX_CHAR_LENGTH_DEFAULT 100000

#define DEFAULTS_SCROLL_TO_BOTTOM_ON_MESSAGE @"MWorksCocoaConsoleScrollToBottomOnMessage"
#define DEFAULTS_ALERT_ON_ERROR @"MWorksCocoaConsoleAlertOnError"


@interface MWConsoleController(PrivateMethods)
- (void)incomingEvent:(MWCocoaEvent *)event;
- (void)scrollToBottom;

- (void)postMessage:(NSString *)msg 
		   atNumber:(NSNumber *)aTime 
			 ofType:(int)type 
	 onLocalConsole:(BOOL)localConsole;
- (void)cacheCodes;
- (void)updateMessageTypeControl:(NSNumber *)messageSegment;
- (void)deleteExcessConsoleMessage;
@end


@implementation MWConsoleController {
    id<MWClientServerBase> delegate;
}


+ (void)initialize {
    //
    // The class identity test ensures that this method is called only once.  For more info, see
    // http://lists.apple.com/archives/cocoa-dev/2009/Mar/msg01166.html
    //
    if (self == [MWConsoleController class]) {
        [[NSUserDefaults standardUserDefaults] registerDefaults:@{ DEFAULTS_SCROLL_TO_BOTTOM_ON_MESSAGE: @YES,
                                                                   DEFAULTS_ALERT_ON_ERROR: @YES }];
    }
}


- (id)init {
    self = [super initWithWindowNibName:@"ConsoleWindow"];
    if(self) {
        fullText = nil;
		maxConsoleLength = MW_CONSOLE_MAX_CHAR_LENGTH_DEFAULT;
        [self window];
        
        //
        // As of macOS 14.1, NSTextView's layout method will eventually go in to some
        // sort of death loop, where it starts trying to allocate an unlimited amount
        // of memory.  The only way to stop it is to quit the application.
        //
        // The following code seems to mitigate (and possibly eliminate) this issue.
        // Per the NSTextView docs: "In macOS 12 and later, if you explicitly call the
        // layoutManager property on a text view or text container, the framework reverts
        // to a compatibility mode that uses NSLayoutManager" (instead of the newer
        // NSTextLayoutManager, which seems to be involved in the death loop).  Since
        // we don't care which layout manager class NSTextView uses, this is a perfectly
        // acceptable workaround.
        //
        NSLayoutManager *layoutManager = msgTextView.layoutManager;
        if (!layoutManager) {
            NSLog(@"MWConsoleController: msgTextView.layoutManager is null (%p)", layoutManager);
        }
        NSTextLayoutManager *textLayoutManager = msgTextView.textLayoutManager;
        if (textLayoutManager) {
            NSLog(@"MWConsoleController: msgTextView.textLayoutManager is non-null (%p)", textLayoutManager);
        }
    }
    
    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
	
	showGenericMessages = YES;
	showErrorMessages = YES;
	showWarningMessages = YES;
    scrollToBottomOnMessage = [defaults boolForKey:DEFAULTS_SCROLL_TO_BOTTOM_ON_MESSAGE];
	grabFocusOnError = [defaults boolForKey:DEFAULTS_ALERT_ON_ERROR];
	grabFocusOnWarning = NO;
	
	[self performSelectorOnMainThread:@selector(updateMessageTypeControl:) 
						   withObject:[NSNumber numberWithInt:MWGenericMessageSegment] 
						waitUntilDone:NO];
	[self performSelectorOnMainThread:@selector(updateMessageTypeControl:) 
						   withObject:[NSNumber numberWithInt:MWWarningMessageSegment] 
						waitUntilDone:NO];
	[self performSelectorOnMainThread:@selector(updateMessageTypeControl:) 
						   withObject:[NSNumber numberWithInt:MWErrorMessageSegment] 
						waitUntilDone:NO];
				
	//	[messageTypeControl setSelected:showGenericMessages 
	//						 forSegment:MWGenericMessageSegment];
	//	[messageTypeControl setSelected:showWarningMessages 
	//						 forSegment:MWWarningMessageSegment];
	//	[messageTypeControl setSelected:showErrorMessages 
	//						 forSegment:MWErrorMessageSegment];
	
	messageCodecCode = -1;
	
	//	[[NSNotificationCenter defaultCenter] addObserver:self
	//											 selector:@selector(incomingEvent:)
	//												 name:MWEventNotification
	//                                               object:nil];
	
    return self;
}


- (id)delegate {
    return delegate;
}

- (void)setDelegate:(id)new_delegate {
	if (![new_delegate respondsToSelector:@selector(codeForTag:)] ||
      ![new_delegate respondsToSelector:@selector(registerEventCallbackWithReceiver:selector:callbackKey:forVariableCode:onMainThread:)]) {
        [NSException raise:NSInternalInconsistencyException
					format:@"Delegate doesn't respond to methods required in MWConsoleController"];
    }
	
	delegate = new_delegate;
	
	[delegate registerEventCallbackWithReceiver:self 
                                     selector:@selector(incomingEvent:)
                                  callbackKey:MW_CONSOLE_CONTROLLER_CALLBACK_KEY
                              forVariableCode:RESERVED_CODEC_CODE
                                 onMainThread:YES];
	
	messageCodecCode = [[delegate codeForTag:[NSString stringWithCString:ANNOUNCE_MESSAGE_VAR_TAGNAME
																encoding:NSASCIIStringEncoding]] intValue];
	
	if(messageCodecCode > 0) {
		
		[delegate registerEventCallbackWithReceiver:self 
                                       selector:@selector(incomingEvent:)
                                    callbackKey:MW_CONSOLE_CONTROLLER_CALLBACK_KEY
                                forVariableCode:messageCodecCode
                                   onMainThread:YES];
	}
}

- (void)windowDidLoad {
    [super windowDidLoad];
	
}

- (void)setTitle:(NSString *)title {
	[[self window] setTitle:title];
}

- (IBAction)setErrorFiltering:(id)sender{
	
	if([messageTypeControl isSelectedForSegment:MWGenericMessageSegment]){
		showGenericMessages = YES;
	} else {
		showGenericMessages = NO;
	}
	
	if([messageTypeControl isSelectedForSegment:MWWarningMessageSegment]){
		showWarningMessages = YES;
	} else {
		showWarningMessages = NO;
	}
	
	if([messageTypeControl isSelectedForSegment:MWErrorMessageSegment]){
		showErrorMessages = YES;
	} else {
		showErrorMessages = NO;
	}
}


- (IBAction)setScrollToBottomOnMessage:(id)sender {
    scrollToBottomOnMessage = ([scrollToBottomSwitch state] == NSControlStateValueOn);
}


- (IBAction)setAlertFocus:(id)sender {
    grabFocusOnError = ([alertSwitch state] == NSControlStateValueOn);
}


/*****************************************************************
*                      Delegate Methods
*****************************************************************/
- (void)saveLog:(id)arg {
	NSString *log;
	@synchronized (msgTextView){
		log = [[NSString alloc] initWithString:[msgTextView string]];
	}	
	
	NSSavePanel * save = [NSSavePanel savePanel];
    save.allowedContentTypes = @[ UTTypePlainText ];
    [save setCanCreateDirectories:YES];
    if([save runModal] == NSModalResponseOK)  {
		[log writeToURL:[save URL]
			  atomically:NO
				encoding:NSASCIIStringEncoding
				   error:nil];
    }
	
}

- (void)clearLog:(id)arg {
	@synchronized (msgTextView){
		[msgTextView setString:@""];
	}
}



/*****************************************************************
*                      Private Methods
*****************************************************************/
- (void)scrollToBottom {
    //[msgTextViewLock lock];
	
	//@synchronized(msgTextView){
		int length = [[msgTextView textStorage] length];
		//[msgTextViewLock unlock];
		//[msgTextView lockFocusIfCanDraw];
		[msgTextView scrollRangeToVisible:NSMakeRange(length, 0)];	
		//[msgTextView unlockFocus];
		//[msgTextView setNeedsDisplay:YES];
	//}
}

- (void)updateMessageTypeControl:(NSNumber *)messageSegment {
	BOOL whichShouldShow = NO;
	
	switch([messageSegment intValue]) {
		case MWGenericMessageSegment:
			whichShouldShow = showGenericMessages;
			break;
		case MWWarningMessageSegment:
			whichShouldShow = showWarningMessages;
			break;
		case MWErrorMessageSegment:
			whichShouldShow = showErrorMessages;
			break;
		default:
			break;
	}
	
	@synchronized(messageTypeControl) {	
		[messageTypeControl setSelected:whichShouldShow 
							 forSegment:[messageSegment intValue]];
	}
}

- (void)postMessage:(NSString *)msg 
		   atNumber:(NSNumber *)aTime
			 ofType:(int)type
	 onLocalConsole:(BOOL)localConsole {
	
    
	MWMessageContainer * msgData;
    msgData = [[MWMessageContainer alloc]
                        initMessageContainerWithMessage:msg 
												 atTime:aTime 
												 ofType:type
										 onLocalConsole:localConsole];
	
	
	//	[[msgTextView textStorage] appendAttributedString:[msgData messageForConsole]];
	[[msgTextView textStorage] performSelectorOnMainThread:@selector(appendAttributedString:)
												withObject:[msgData messageForConsole]
											 waitUntilDone:YES];
	
	
	if([[msgTextView textStorage] length] > maxConsoleLength*1.5) {
		[self performSelectorOnMainThread:@selector(deleteExcessConsoleMessage)
							   withObject:[NSNumber numberWithInt:[[msgTextView textStorage] length]]
							waitUntilDone:YES];
		
		
		
		
//		/*#define MW_CONSOLE_UPDATE_INTERVAL	10
//		static MWorksTime last_time = 0; 
//		MWorksTime	current_time = GlobalClock->getCurrentTimeMS();
//		if(current_time - last_time > MW_CONSOLE_UPDATE_INTERVAL){*/
//		/*[[NSNotificationCenter defaultCenter] 
//postNotificationName:MWConsoleUpdatedNotification 
//object:nil userInfo:nil];*/
//		
//		/*	last_time = current_time;
//		}*/
	}
	
	if (scrollToBottomOnMessage) {
		[self performSelectorOnMainThread:@selector(scrollToBottom)
							   withObject:nil
							waitUntilDone:YES];
	}
}

- (void)incomingEvent:(MWCocoaEvent *)event {
	int code = [event code];
	
	if(code == RESERVED_CODEC_CODE) {
		[self performSelectorOnMainThread:@selector(cacheCodes) withObject:Nil waitUntilDone:NO];
        //[self cacheCodes];
	} else {
		
		if (code == messageCodecCode) {
			//mData *pl = [event data];
			Datum payload(*[event data]);
			
			if(payload.getDataType() != M_DICTIONARY){
				return;
			}
			
			if(payload.getElement(M_MESSAGE).getDataType() != M_STRING){
				return;
			}
			
			string message_str(payload.getElement(M_MESSAGE).getString());
			if(message_str.empty()){
				return;
			}
			//NSString *message = [[[NSString alloc] initWithCString:message_str.c_str() 
//														encoding:NSASCIIStringEncoding] autorelease];
			NSString *message = [NSString stringWithCString:message_str.c_str() encoding:NSASCIIStringEncoding];
      
			int msgType = payload.getElement(M_MESSAGE_TYPE).getInteger();
			
			MWorksTime eventTime = [event time];
			
			NSNumber *time = [[NSNumber alloc] initWithLongLong:eventTime];
			
            BOOL localConsole = (payload.getElement(M_MESSAGE_ORIGIN).getInteger() == M_SERVER_MESSAGE_ORIGIN);
			if(!showGenericMessages && msgType == M_GENERIC_MESSAGE){
				return;
			}
			
			if(!showWarningMessages && msgType == M_WARNING_MESSAGE){
				return;
			}
			
			if(!showErrorMessages && msgType == M_ERROR_MESSAGE){
				return;
			}
			
			[self postMessage:message 
					 atNumber:time
					   ofType:msgType
			   onLocalConsole:localConsole];
			
			if(grabFocusOnError && msgType == M_ERROR_MESSAGE){
				if(![[self window] isVisible]){
					[self performSelectorOnMainThread:@selector(showWindow:)
										   withObject:self
										waitUntilDone:NO];
					//				[self showWindow:self];
				}
				// If we're grabing focus, we should probably be sure to actually
				// show the error that grabbed the focus
				showErrorMessages = YES;
				[self performSelectorOnMainThread:@selector(updateMessageTypeControl:) 
									   withObject:[NSNumber numberWithInt:MWErrorMessageSegment] 
									waitUntilDone:NO];
				//				[messageTypeControl setSelected:showErrorMessages 
				//									 forSegment:MWGenericMessageSegment];
			}
			
			if(grabFocusOnWarning && msgType == M_WARNING_MESSAGE){
				if(![[self window] isVisible]){
					[self performSelectorOnMainThread:@selector(showWindow:)
										   withObject:self
										waitUntilDone:NO];
					//				[self showWindow:self];
				}
			}
		}
	}
}

- (void)cacheCodes {
	if(delegate != nil) {
		[delegate unregisterCallbacksWithKey:MW_CONSOLE_CONTROLLER_CALLBACK_KEY];
		
		[delegate registerEventCallbackWithReceiver:self 
                                       selector:@selector(incomingEvent:)
                                    callbackKey:MW_CONSOLE_CONTROLLER_CALLBACK_KEY
                                forVariableCode:RESERVED_CODEC_CODE
                                   onMainThread:YES];
		
		messageCodecCode = [[delegate codeForTag:[NSString stringWithCString:ANNOUNCE_MESSAGE_VAR_TAGNAME
																	encoding:NSASCIIStringEncoding]] intValue];
		
		if(messageCodecCode > 0) {
			
			[delegate registerEventCallbackWithReceiver:self 
                                         selector:@selector(incomingEvent:)
                                      callbackKey:MW_CONSOLE_CONTROLLER_CALLBACK_KEY
                                  forVariableCode:messageCodecCode
                                     onMainThread:YES];
		}
	}
}

- (void)deleteExcessConsoleMessage {
	[[msgTextView textStorage] deleteCharactersInRange:NSMakeRange(0, 
																   [[msgTextView textStorage] length] - maxConsoleLength)];
}

@end

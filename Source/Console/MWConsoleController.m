//
//  MWConsoleController.m
//  MonkeyWorksGUIComponents
//
//  Created by Ben Kennedy on 7/23/07.
//  Copyright 2007 MIT. All rights reserved.
//

#import "MWMessageContainer.h"
#import "MWConsoleController.h"
#import "MWNotifications.h"
#import "MWCocoaEvent.h"
#import "MonkeyWorksCore/StandardVariables.h"
#import "MWClientServerBase.h"

#define MW_CONSOLE_CONTROLLER_CALLBACK_KEY @"MonkeyWorksCocoa console controller callback key"
#define MW_CONSOLE_MAX_CHAR_LENGTH_DEFAULT 10000


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


@implementation MWConsoleController

- (id)init {	
	return [self initWithShowGeneric:YES showWarning:YES showError:YES 
					grabFocusOnError:YES];
}

- (id)initWithShowGeneric:(BOOL)show_generic 
			  showWarning:(BOOL)show_warning
				showError:(BOOL)show_error 
		 grabFocusOnError:(BOOL)grab_on_error {
	
    self = [super initWithWindowNibName:@"ConsoleWindow"];
    if(self) {
        fullText = nil;
		maxConsoleLength = MW_CONSOLE_MAX_CHAR_LENGTH_DEFAULT;
        [self window];
    }
	
	showGenericMessages = show_generic;
	showErrorMessages = show_error;
	showWarningMessages = show_warning;
	grabFocusOnError = grab_on_error;
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
	
	[alertSwitch setState:grabFocusOnError];
	
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
		![new_delegate respondsToSelector:@selector(maxConsoleLength)] ||
		![new_delegate respondsToSelector:@selector(isLocalMessage:)] ||
		![new_delegate respondsToSelector:@selector(registerEventCallbackWithRecevier:
													andSelector:
													andKey:)]) {
        [NSException raise:NSInternalInconsistencyException
					format:@"Delegate doesn't respond to methods required in MWConsoleController"];
    }
	
	delegate = new_delegate;
	
	[delegate registerEventCallbackWithRecevier:self 
									andSelector:@selector(incomingEvent:)
										 andKey:MW_CONSOLE_CONTROLLER_CALLBACK_KEY
								forVariableCode:[NSNumber numberWithInt:RESERVED_CODEC_CODE]];
	
	messageCodecCode = [[delegate codeForTag:[NSString stringWithCString:ANNOUNCE_MESSAGE_VAR_TAGNAME
																encoding:NSASCIIStringEncoding]] intValue];
	
	if(messageCodecCode > 0) {
		
		[delegate registerEventCallbackWithRecevier:self 
										andSelector:@selector(incomingEvent:)
											 andKey:MW_CONSOLE_CONTROLLER_CALLBACK_KEY
									forVariableCode:[NSNumber numberWithInt:messageCodecCode]];
	}
	
	maxConsoleLength = [[delegate maxConsoleLength] intValue];
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

- (IBAction)setAlertFocus:(id)sender{
	
	if([alertSwitch state] == NSOnState){
		grabFocusOnError = YES;
	} else {
		grabFocusOnError = NO;
	}
}

/*****************************************************************
*                      Delegate Methods
*****************************************************************/
- (void)saveLog:(id)arg {
	NSString *log;
	@synchronized (msgTextView){
		log = [[NSString alloc] initWithString:[msgTextView string]];
	}	
	
	NSSavePanel * save = [[NSSavePanel savePanel] retain];
    [save setAllowedFileTypes:[NSArray arrayWithObject:@"txt"]];
    [save setCanCreateDirectories:YES];
    if([save runModalForDirectory:nil file:nil] ==
	   NSFileHandlingPanelOKButton)  {
		[log writeToFile:[save filename] 
			  atomically:NO
				encoding:NSASCIIStringEncoding
				   error:nil];
    }
	
	[save release];
	[log release];
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
		int length = [[[msgTextView textStorage] string] length];
		//[msgTextViewLock unlock];
		//[msgTextView lockFocusIfCanDraw];
		[msgTextView scrollRangeToVisible:NSMakeRange(length, 1)];	
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
											 waitUntilDone:NO];
	[msgData release];
	
	
	if([[msgTextView textStorage] length] > maxConsoleLength*1.5) {
		[self performSelectorOnMainThread:@selector(deleteExcessConsoleMessage)
							   withObject:[NSNumber numberWithInt:[[msgTextView textStorage] length]]
							waitUntilDone:NO];
		
		
		
		
//		/*#define MW_CONSOLE_UPDATE_INTERVAL	10
//		static MonkeyWorksTime last_time = 0; 
//		MonkeyWorksTime	current_time = GlobalClock->getCurrentTimeMS();
//		if(current_time - last_time > MW_CONSOLE_UPDATE_INTERVAL){*/
//		/*[[NSNotificationCenter defaultCenter] 
//postNotificationName:MWConsoleUpdatedNotification 
//object:nil userInfo:nil];*/
//		
//		/*	last_time = current_time;
//		}*/
	}
	
	if([scrollToBottomSwitch state] == NSOnState) {		
		
		[self performSelectorOnMainThread:@selector(scrollToBottom)
							   withObject:nil
							waitUntilDone:YES];
	}
}

- (void)incomingEvent:(MWCocoaEvent *)event {
	int code = [event code];
	
	if(code == RESERVED_CODEC_CODE) {
		[self cacheCodes];
	} else {
		
		if (code == messageCodecCode) {
			//mData *pl = [event data];
			Data payload(*[event data]);
			
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
			NSString *message = [[[NSString alloc] initWithCString:message_str.c_str() 
														encoding:NSASCIIStringEncoding] autorelease];
			
			int msgType = payload.getElement(M_MESSAGE_TYPE).getInteger();
			
			MonkeyWorksTime eventTime = [event time];
			
			NSNumber *time = [[[NSNumber alloc] initWithLongLong:eventTime/1000] autorelease];
			
			NSNumber *localConsole = [delegate isLocalMessage:[NSNumber numberWithInt:payload.getElement(M_MESSAGE_ORIGIN).getInteger()]];
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
			   onLocalConsole:[localConsole boolValue]];
			
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
		
		[delegate registerEventCallbackWithRecevier:self 
										andSelector:@selector(incomingEvent:)
											 andKey:MW_CONSOLE_CONTROLLER_CALLBACK_KEY
									forVariableCode:[NSNumber numberWithInt:RESERVED_CODEC_CODE]];
		
		messageCodecCode = [[delegate codeForTag:[NSString stringWithCString:ANNOUNCE_MESSAGE_VAR_TAGNAME
																	encoding:NSASCIIStringEncoding]] intValue];
		
		if(messageCodecCode > 0) {
			
			[delegate registerEventCallbackWithRecevier:self 
											andSelector:@selector(incomingEvent:)
												 andKey:MW_CONSOLE_CONTROLLER_CALLBACK_KEY
										forVariableCode:[NSNumber numberWithInt:messageCodecCode]];
		}
	}
}

- (void)deleteExcessConsoleMessage {
	[[msgTextView textStorage] deleteCharactersInRange:NSMakeRange(0, 
																   [[msgTextView textStorage] length] - maxConsoleLength)];
}

@end

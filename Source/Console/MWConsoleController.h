//
//  MWConsoleController.h
//  MonkeyWorksGUIComponents
//
//  Created by Ben Kennedy on 7/23/07.
//  Copyright 2007 MIT. All rights reserved.
//

#ifndef MW_CONSOLE_CONTROLLER_H_
#define MW_CONSOLE_CONTROLLER_H_

#import <Cocoa/Cocoa.h>

const int MWGenericMessageSegment = 0;
const int MWWarningMessageSegment = 2;
const int MWErrorMessageSegment = 1;

@interface MWConsoleController : NSWindowController {
	IBOutlet NSTextView *msgTextView;
    IBOutlet NSSegmentedControl *messageTypeControl;
	IBOutlet NSButton *alertSwitch;
	IBOutlet NSButton *scrollToBottomSwitch;
	
	BOOL firstSearch;
    int fileCode;
	
    NSAttributedString * fullText;
	BOOL showGenericMessages;
	BOOL showWarningMessages;
	BOOL showErrorMessages;
	BOOL grabFocusOnWarning;
	BOOL grabFocusOnError;
	
	int messageCodecCode;
	int maxConsoleLength;
	
	id delegate;
}

- (id)init;

- (id)initWithShowGeneric:(BOOL)show_generic showWarning:(BOOL)show_warning
				showError:(BOOL)show_error 
		 grabFocusOnError:(BOOL)grab_on_error;

- (id)delegate;
- (void)setDelegate:(id)new_delegate;
- (void)setTitle:(NSString *)title;

- (IBAction)setErrorFiltering:(id)sender;
- (IBAction)setAlertFocus:(id)sender;

@end

@interface MWConsoleController(DelegateMethods)
- (void)saveLog:(id)arg;
- (void)clearLog:(id)arg;
@end

#endif


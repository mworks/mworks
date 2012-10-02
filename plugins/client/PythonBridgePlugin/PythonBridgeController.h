//
//  PythonBridgePlugin.h
//  PythonBridgePlugin
//
//  Created by David Cox on 12/21/09.
//  Copyright 2009 Harvard University. All rights reserved.
//

#import <MWorksCocoa/MWWindowController.h>
#import <MWorksCocoa/MWClientProtocol.h>

#import <MWorksCore/Client.h>
#import <MWorksCore/EventStreamConduit.h>
#import <MWorksCocoa/MWCoreContainer.h>

@interface PythonBridgeController : NSWindowController {

    IBOutlet id<MWClientProtocol> delegate;
    
    // The "core" object and a conduit that will be connected to the 
    // external script process
    shared_ptr<mw::EventStreamInterface> core;
    shared_ptr<mw::EventStreamConduit> conduit;

    // The python task / process
    NSTask *python_task;
    NSFileHandle *python_task_stdout;
    NSFileHandle *python_task_stderr;
    NSPipe *stdout_pipe, *stderr_pipe;
    
    
    // A timer to check the stdout / viability of the task
    NSTimer *task_check_timer;
    
    // Variables for bindings
    NSString *path;
    NSString *status;
    NSString *loadButtonTitle;
    BOOL scrollToBottomOnOutput;
    
    // GUI view objects
    IBOutlet NSView *content_view;
    IBOutlet NSTextView *console_view;
    IBOutlet NSWindow *script_chooser_sheet;
    IBOutlet NSProgressIndicator *working_indicator;
    IBOutlet NSPopUpButton *recent_scripts;

    BOOL in_grouped_window;
}

@property(retain) NSString *path;
@property(retain) NSString *status;
@property(retain) NSString *loadButtonTitle;
@property(nonatomic, assign) BOOL scrollToBottomOnOutput;


-(void) awakeFromNib;

-(void)initConduit;
-(void)launchScriptAtPath:(NSString *)path;
-(void)launchScriptChooserSheet;
-(void)terminateScript;

-(IBAction)loadButtonPress:(id)sender;

- (IBAction)closeScriptChooserSheet:(id)sender;
- (IBAction)chooseScript:(id)sender;
- (IBAction)launchRecentScript:(id)sender;


- (void)checkOnPythonTask;

- (void) updateRecentScripts;

- (void) setInGroupedWindow:(BOOL)isit;

@end

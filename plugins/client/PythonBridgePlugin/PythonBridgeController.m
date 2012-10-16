//
//  PythonBridgePlugin.m
//  PythonBridgePlugin
//
//  Created by David Cox on 12/21/09.
//  Copyright 2009 Harvard University. All rights reserved.
//

#import "PythonBridgeController.h"

#define CONDUIT_RESOURCE_NAME  "python_bridge_plugin_conduit"
#define LOAD_BUTTON_TITLE   @"choose..."
#define TERMINATE_BUTTON_TITLE  @"terminate"
#define STATUS_LOADING  @"Loading..."
#define STATUS_NONE_LOADED @"None loaded"
#define STATUS_ACTIVE   @"Active"
#define STATUS_TERMINATING  @"Terminating..."

#define DEFAULTS_SCROLL_TO_BOTTOM_ON_OUTPUT_KEY @"autoScrollPythonOutput"

#ifdef __x86_64__
#  define PYTHON_ARCH @"x86_64"
#else
#  define PYTHON_ARCH @"i386"
#endif

#import <MWorksCore/IPCEventTransport.h>

@implementation PythonBridgeController


@synthesize path;
@synthesize status;
@synthesize loadButtonTitle;
@synthesize scrollToBottomOnOutput;


-(void)awakeFromNib {
    
    [self setLoadButtonTitle:LOAD_BUTTON_TITLE];
    [self setPath:Nil];
    [self setStatus:STATUS_NONE_LOADED];
    in_grouped_window = NO;
    self.scrollToBottomOnOutput = [[NSUserDefaults standardUserDefaults]
                                   boolForKey:DEFAULTS_SCROLL_TO_BOTTOM_ON_OUTPUT_KEY];
    
    // Automatically terminate script at application shutdown
    [[NSNotificationCenter defaultCenter] addObserverForName:NSApplicationWillTerminateNotification
                                                      object:nil
                                                       queue:[NSOperationQueue mainQueue]
                                                  usingBlock:^(NSNotification *notification) {
                                                      [self terminateScript];
                                                  }];
}

- (void) setInGroupedWindow:(BOOL)isit {
    in_grouped_window = isit;
}


- (void)setScrollToBottomOnOutput:(BOOL)shouldScroll {
    if (scrollToBottomOnOutput != shouldScroll) {
        [self willChangeValueForKey:@"scrollToBottomOnOutput"];
        scrollToBottomOnOutput = shouldScroll;
        [self didChangeValueForKey:@"scrollToBottomOnOutput"];
        [[NSUserDefaults standardUserDefaults] setBool:shouldScroll forKey:DEFAULTS_SCROLL_TO_BOTTOM_ON_OUTPUT_KEY];
    }
}


- (IBAction)launchRecentScript:(id)sender{
 
    path = [recent_scripts titleOfSelectedItem];
    [self launchScriptAtPath:path];
    [self closeScriptChooserSheet:self];
}

-(void)initConduit {
    core = [(id<MWCoreContainer>)delegate eventStreamInterface];
    
    // TODO: generate a unique name to avoid name collisions
    shared_ptr<mw::IPCEventTransport> transport(new mw::IPCEventTransport(mw::EventTransport::server_event_transport, 
                                                                          mw::EventTransport::bidirectional_event_transport, 
                                                                          CONDUIT_RESOURCE_NAME));
    
    // build the conduit, attaching it to the core/client's event stream 
    conduit = shared_ptr<mw::EventStreamConduit>(new mw::EventStreamConduit(transport, core));
    conduit->initialize();
}

-(void)launchScriptAtPath:(NSString *)script_path{
    if (!conduit) {
        [self initConduit];
    }
    
    [self setPath:script_path];
    
    python_task = [[NSTask alloc] init];
    [python_task setLaunchPath: @"/usr/bin/arch"];
    
    NSArray *arguments;
    arguments = [NSArray arrayWithObjects:@"-arch", PYTHON_ARCH,
                 @"/usr/bin/python" PYTHON_VERSION,
                 script_path,
                 [NSString stringWithCString:CONDUIT_RESOURCE_NAME encoding:NSASCIIStringEncoding],
                 nil];
    [python_task setArguments: arguments];
    
    stdout_pipe = [NSPipe pipe];
    stderr_pipe = [NSPipe pipe];
    [python_task setStandardOutput: stdout_pipe];
    [python_task setStandardError: stderr_pipe];
    
    python_task_stdout = [stdout_pipe fileHandleForReading];
    python_task_stderr = [stderr_pipe fileHandleForReading];
    
    
    [self updateRecentScripts];
    [python_task launch];
    
    [self setLoadButtonTitle:TERMINATE_BUTTON_TITLE];
    
    [working_indicator startAnimation:self];
    [self setStatus:STATUS_LOADING];
    
    // Register notifications so that we can get stdout and stderr
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(postDataFromStdout:) name:NSFileHandleReadCompletionNotification object:python_task_stdout];
    [python_task_stdout readInBackgroundAndNotify];
    
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(postDataFromStderr:) name:NSFileHandleReadCompletionNotification object:python_task_stderr];
    [python_task_stderr readInBackgroundAndNotify];
    
    
    // start a timer to check on the task
    task_check_timer = [NSTimer timerWithTimeInterval:0.5 target:self selector:@selector(checkOnPythonTask) userInfo:Nil repeats:YES]; 
    [[NSRunLoop currentRunLoop] addTimer:task_check_timer forMode:NSDefaultRunLoopMode];
    
    
}

-(void)launchScriptChooserSheet{
    
    NSWindow *parent_window;
    
    if(in_grouped_window){
        parent_window = [delegate groupedPluginWindow];
    } else {
        parent_window = [self window];
    }
    
    [NSApp beginSheet: script_chooser_sheet
       modalForWindow: parent_window
        modalDelegate: self
       didEndSelector: @selector(didEndSheet:returnCode:contextInfo:)
          contextInfo: nil];
    
}

- (void)didEndSheet:(NSWindow *)sheet returnCode:(int)returnCode contextInfo:(void *)contextInfo
{
    [sheet orderOut:Nil];
}

- (IBAction)closeScriptChooserSheet:(id)sender {
    [NSApp endSheet:script_chooser_sheet];
}


- (IBAction)chooseScript:(id)sender {
    // Create the File Open Dialog class.
    NSOpenPanel* openDlg = [NSOpenPanel openPanel];
    
    // Enable the selection of files in the dialog.
    [openDlg setCanChooseFiles:YES];
    [openDlg setCanChooseDirectories:NO];
    [openDlg setAllowsMultipleSelection:NO];
    
    
    // Display the dialog.  If the OK button was pressed,
    // process the files.
    if ( [openDlg runModal] == NSFileHandlingPanelOKButton )
    {
        // Get an array containing the full filenames of all
        // files and directories selected.
        NSArray* files = [openDlg URLs];
        
        if([files count] != 1){
            // TODO: raise hell
            return;
        }
        
        // Loop through all the files and process them.
        for(int i = 0; i < [files count]; i++ )
        {
            NSString* file_name = [[files objectAtIndex:i] path];
            
            if(file_name == Nil){
                return;
            }
            
            path = file_name;
            [self launchScriptAtPath:path];
            [self closeScriptChooserSheet:self];
            
        }
    }
    
}


- (void)postToConsole:(NSAttributedString *)attstr {
    [[console_view textStorage] appendAttributedString:attstr];
    if (self.scrollToBottomOnOutput) {
        [console_view scrollRangeToVisible:NSMakeRange([[console_view textStorage] length], 0) ];
    }
}


- (void) postDataFromStdout:(id)notification{
    
    NSData *data = [[notification userInfo] objectForKey:NSFileHandleNotificationDataItem];
    
    if([data length] != 0){
        NSString *str;
        str = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
        NSAttributedString *attstr = [[NSAttributedString alloc] initWithString:str];
        
        [self postToConsole:attstr];
        
        // reregister a request to read in the background
        [python_task_stdout readInBackgroundAndNotify];
    }
}


- (void) postDataFromStderr:(id)notification{
    
    NSData *data = [[notification userInfo] objectForKey:NSFileHandleNotificationDataItem];
    
    if([data length] != 0){
        
        NSString *str;
        str = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
        NSDictionary *attr = [NSDictionary dictionaryWithObject:[NSColor redColor] 
                                                         forKey:NSForegroundColorAttributeName];
        
        NSAttributedString *attstr = [[NSAttributedString alloc] initWithString:str attributes:attr];
        
        [self postToConsole:attstr];
        
        // reregister a request to read in the background
        [python_task_stderr readInBackgroundAndNotify];
    }
    
    
}


- (void)checkOnPythonTask{
    
    if(python_task != Nil && [python_task isRunning]){
        [working_indicator stopAnimation:self];
    }
    
    if(python_task == Nil || ![python_task isRunning]){
        [self setStatus:STATUS_NONE_LOADED];
        [self setLoadButtonTitle:LOAD_BUTTON_TITLE];
        [task_check_timer invalidate];
        python_task = Nil;
    }
    
    
    
    // read the stderr pipe
    //data = [python_task_stderr readDataToEndOfFile];
    //data = [python_task_stderr availableData];
//    str = [[NSString alloc] initWithData: data encoding: NSUTF8StringEncoding];
//    
//    NSDictionary *attr = [NSDictionary dictionaryWithObject:[NSColor redColor] 
//                                                     forKey:NSForegroundColorAttributeName];
//    
//    attstr = [[NSAttributedString alloc] initWithString:str 
//                                             attributes:attr];
//    
//    [[console_view textStorage] appendAttributedString:attstr];
//    
}


-(void)terminateScript{
    if(python_task != Nil){
        [self setStatus:STATUS_TERMINATING];
        [python_task terminate];
    }
    
    python_task = Nil;
    python_task_stdout = Nil;
    
    [self setStatus:STATUS_NONE_LOADED];
    [self setLoadButtonTitle:LOAD_BUTTON_TITLE];
}

-(IBAction)loadButtonPress:(id)sender{

    if(python_task == Nil){
        [self launchScriptChooserSheet];
    } else if(python_task != Nil){
        [self terminateScript];
    } else {
        // this is some kind of error
        NSLog(@"some kind of error occurred");
    }
}


- (void) updateRecentScripts {
	NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    
    if([self path] != nil){
        [defaults setObject:[self path] forKey:@"lastPythonScript"];
        [defaults synchronize];
    }
    NSArray *recentScripts = [defaults arrayForKey:@"recentPythonScripts"];
    if([self path] != nil){
        NSMutableArray *recentScriptsMutable = [NSMutableArray arrayWithArray:recentScripts];
        [recentScriptsMutable removeObject:[self path]];  // In case it's already in the list
        [recentScriptsMutable insertObject:[self path] atIndex:0];
        [defaults setObject:recentScriptsMutable forKey:@"recentPythonScripts"];
        [defaults synchronize];
    }
}

@end






















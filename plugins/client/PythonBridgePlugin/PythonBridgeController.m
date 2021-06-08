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
#define STATUS_ACTIVE   @"Active"
#define STATUS_TERMINATING  @"Terminating..."

#define DEFAULTS_PYTHON_EXECUTABLE_KEY @"pythonExecutable"
#define DEFAULTS_SCROLL_TO_BOTTOM_ON_OUTPUT_KEY @"autoScrollPythonOutput"

#define MAX_NUM_RECENT_SCRIPTS 20

#import <MWorksCocoa/NSString+MWorksCocoaAdditions.h>
#import <MWorksCore/ZeroMQIPCEventTransport.hpp>

@implementation PythonBridgeController


@synthesize path;
@synthesize status;
@synthesize loadButtonTitle;
@synthesize scrollToBottomOnOutput;


-(void)awakeFromNib {
    
    [self setLoadButtonTitle:LOAD_BUTTON_TITLE];
    self.path = nil;
    self.status = nil;
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


- (void)launchScriptFromScriptChooserSheet:(NSString *)path {
    if ([self launchScriptAtPath:path]) {
        [self closeScriptChooserSheet:self];
    }
}


- (IBAction)launchRecentScript:(id)sender {
    path = [recent_scripts titleOfSelectedItem];
    [self launchScriptFromScriptChooserSheet:path];
}

-(void)initConduit {
    core = [delegate coreClient];
    
    // TODO: generate a unique name to avoid name collisions
    auto transport = boost::make_shared<mw::ZeroMQIPCEventTransport>(mw::EventTransport::server_event_transport,
                                                                     CONDUIT_RESOURCE_NAME);
    
    // build the conduit, attaching it to the core/client's event stream 
    conduit = shared_ptr<mw::EventStreamConduit>(new mw::EventStreamConduit(transport, core));
    conduit->initialize();
}

- (BOOL)launchScriptAtPath:(NSString *)script_path{
    NSString *pythonExecutable = [NSUserDefaults.standardUserDefaults stringForKey:DEFAULTS_PYTHON_EXECUTABLE_KEY];
    
    if (!pythonExecutable) {
        NSAlert *alert = [[NSAlert alloc] init];
        alert.messageText = @"No Python executable selected";
        alert.informativeText = @"Please specify the Python executable to invoke when running the script.";
        if (script_chooser_sheet.sheetParent) {
            [alert beginSheetModalForWindow:script_chooser_sheet completionHandler:nil];
        } else {
            [alert runModal];
        }
        return NO;
    }
    
    if (!conduit) {
        [self initConduit];
    }
    
    [self setPath:script_path];
    
    python_task = [[NSTask alloc] init];
    [python_task setLaunchPath:pythonExecutable];
    
    NSArray *arguments;
    arguments = [NSArray arrayWithObjects:
                 script_path,
                 [NSString stringWithCString:CONDUIT_RESOURCE_NAME encoding:NSASCIIStringEncoding],
                 nil];
    [python_task setArguments: arguments];
    
    NSMutableDictionary<NSString *, NSString *> *environment = [NSProcessInfo.processInfo.environment mutableCopy];
    environment[@"PYTHONPATH"] = @"/Library/Application Support/MWorks/Scripting/Python";
    python_task.environment = environment;
    
    stdout_pipe = [NSPipe pipe];
    stderr_pipe = [NSPipe pipe];
    [python_task setStandardOutput: stdout_pipe];
    [python_task setStandardError: stderr_pipe];
    
    python_task_stdout = [stdout_pipe fileHandleForReading];
    python_task_stderr = [stderr_pipe fileHandleForReading];
    
    [self updateRecentScripts];
    [python_task launch];
    
    [self setLoadButtonTitle:TERMINATE_BUTTON_TITLE];
    [self setStatus:STATUS_LOADING];
    
    // Register notifications so that we can get stdout and stderr
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(postDataFromStdout:) name:NSFileHandleReadCompletionNotification object:python_task_stdout];
    [python_task_stdout readInBackgroundAndNotify];
    
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(postDataFromStderr:) name:NSFileHandleReadCompletionNotification object:python_task_stderr];
    [python_task_stderr readInBackgroundAndNotify];
    
    
    // start a timer to check on the task
    task_check_timer = [NSTimer timerWithTimeInterval:0.5 target:self selector:@selector(checkOnPythonTask) userInfo:Nil repeats:YES]; 
    [[NSRunLoop currentRunLoop] addTimer:task_check_timer forMode:NSDefaultRunLoopMode];
    
    return YES;
}


-(void)launchScriptChooserSheet {
    NSWindow *parent_window;
    if (in_grouped_window) {
        parent_window = [delegate groupedPluginWindow];
    } else {
        parent_window = [self window];
    }
    [parent_window beginSheet:script_chooser_sheet completionHandler:nil];
}


- (IBAction)closeScriptChooserSheet:(id)sender {
    [script_chooser_sheet.sheetParent endSheet:script_chooser_sheet];
}


- (IBAction)choosePythonExecutable:(id)sender {
    NSOpenPanel *openPanel = [NSOpenPanel openPanel];
    openPanel.canChooseFiles = YES;
    openPanel.canChooseDirectories = NO;
    openPanel.resolvesAliases = NO;
    openPanel.allowsMultipleSelection = NO;
    openPanel.showsHiddenFiles = YES;
    openPanel.treatsFilePackagesAsDirectories = YES;
    
    if ([openPanel runModal] == NSModalResponseOK) {
        NSString *path = openPanel.URLs.firstObject.path;
        if (path) {
            [NSUserDefaults.standardUserDefaults setObject:path forKey:DEFAULTS_PYTHON_EXECUTABLE_KEY];
        }
    }
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
    if ( [openDlg runModal] == NSModalResponseOK )
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
            [self launchScriptFromScriptChooserSheet:path];
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
        NSAttributedString *attstr = [[NSAttributedString alloc] initWithString:str
                                                                     attributes:@{ NSForegroundColorAttributeName: [NSColor textColor] }];
        
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
    if(python_task == Nil || ![python_task isRunning]){
        self.path = nil;
        self.status = nil;
        [self setLoadButtonTitle:LOAD_BUTTON_TITLE];
        [task_check_timer invalidate];
        python_task = Nil;
    } else {
        self.status = STATUS_ACTIVE;
    }
}


-(void)terminateScript{
    if(python_task != Nil){
        [self setStatus:STATUS_TERMINATING];
        [python_task terminate];
    }
    
    python_task = Nil;
    python_task_stdout = Nil;
    
    if (conduit) {
        conduit->finalize();
        conduit.reset();
    }
    
    self.path = nil;
    self.status = nil;
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
        
        NSArray *recentScripts = [defaults arrayForKey:@"recentPythonScripts"];
        NSMutableArray *recentScriptsMutable;
        if (recentScripts) {
            recentScriptsMutable = [recentScripts mutableCopy];
        } else {
            recentScriptsMutable = [NSMutableArray array];
        }
        [recentScriptsMutable removeObject:[self path]];  // In case it's already in the list
        [recentScriptsMutable insertObject:[self path] atIndex:0];
        while (recentScriptsMutable.count > MAX_NUM_RECENT_SCRIPTS) {
            [recentScriptsMutable removeLastObject];
        }
        [defaults setObject:recentScriptsMutable forKey:@"recentPythonScripts"];
        
        [defaults synchronize];
    }
}


- (NSDictionary *)workspaceState {
    NSMutableDictionary *workspaceState = [NSMutableDictionary dictionary];
    
    NSString *pythonExecutable = [NSUserDefaults.standardUserDefaults stringForKey:DEFAULTS_PYTHON_EXECUTABLE_KEY];
    if (pythonExecutable) {
        workspaceState[@"pythonExecutable"] = pythonExecutable;
    }
    
    if (python_task) {
        [workspaceState setObject:self.path forKey:@"scriptPath"];
    }
    
    return workspaceState;
}


- (void)setWorkspaceState:(NSDictionary *)workspaceState {
    NSString *newPythonExecutable = workspaceState[@"pythonExecutable"];
    if (newPythonExecutable && [newPythonExecutable isKindOfClass:[NSString class]]) {
        [NSUserDefaults.standardUserDefaults setObject:newPythonExecutable forKey:DEFAULTS_PYTHON_EXECUTABLE_KEY];
    }
    
    NSString *newPath = [workspaceState objectForKey:@"scriptPath"];
    if (newPath && [newPath isKindOfClass:[NSString class]]) {
        if (python_task) {
            [self terminateScript];
        }
        [self launchScriptAtPath:[newPath mwk_absolutePath]];
    }
}


@end

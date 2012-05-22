//
//  MWGroupedPluginWindowController.m
//  New Client
//
//  Created by David Cox on 5/29/08.
//  Copyright 2008 Harvard University. All rights reserved.
//

#import "MWGroupedPluginWindowController.h"

#import <MWorksCocoa/MWWindowController.h>


@interface MWGroupedPluginWindowController(PrivateMethods)
-(void) relinquishCurrentView;
@end


@implementation MWGroupedPluginWindowController


@synthesize currentPluginIndex;
@synthesize plugins;
@synthesize names;
@synthesize cycleTime;
@synthesize cycling;
@synthesize clientInstance;

- (id)initWithClientInstance:(id)instance {
    
    plugins = [[NSMutableArray alloc] init];
    names = [[NSMutableArray alloc] init];
    holdFlags = [[NSMutableArray alloc] init];
    currentPluginIndex = 0;
    self.clientInstance = instance;
    self.cycleTime = 5;
    self.cycling = NO;
    cycleTimer = Nil;
    
    return self;
}

- (void) addPluginWindow:(NSWindow *)window withName:(NSString *)name{

    
    [plugins addObject:window];
    [names addObject:name];
    [holdFlags addObject:[NSNumber numberWithBool:NO]];
    
    [self setPlugins:plugins];
    [self setNames:names];
    //[plugins addObject:window];
    //[names addObject:name];
}

- (void) setCurrentPluginIndex:(int)i {

    [self changeViewToPlugin:i];
    currentPluginIndex = i;
}


- (void)hideWindow {

    [self relinquishCurrentView];
    [[self window] orderOut:self];
}

- (void) relinquishCurrentView {
    if(currentPluginIndex > [holdFlags count]){
      return;
    }
    if([[holdFlags objectAtIndex:currentPluginIndex] boolValue]){
        NSWindow *current_content_window = [plugins objectAtIndex:currentPluginIndex];
        
        @try {
            // Make sure that we end all editing in this window, or we could get in trouble
            [current_content_window endEditingFor:Nil];
        }
        @catch (NSException *exception) {
            // FIXME: Doing "Show All", "Show Grouped", and then "Show All" again leads to an exception in
            // endEditingFor.  However, the reason for this exception is a mystery.  Until someone figures out why
            // it's happening, we'll just log the exception and carry on.
            NSLog(@"Ignoring exception in %s: %@", __func__, exception);
        }
        
        [current_content_window setContentView:[content_box contentView]];
        [holdFlags replaceObjectAtIndex:currentPluginIndex withObject:[NSNumber numberWithBool:NO]];
    }
}

- (void) changeViewToPlugin:(int)i{

    
    if(i >= 0 && i < [plugins count]){
        [self relinquishCurrentView];
        
        NSWindow *content_window = [plugins objectAtIndex:i];
        [content_window orderOut:self];
        
        if([[content_window windowController] respondsToSelector:@selector(setInGroupedWindow:)]){
            [(id<MWWindowController>)[content_window windowController] setInGroupedWindow:YES];
        }
        
        NSView *current_view = [content_window contentView];
        [holdFlags replaceObjectAtIndex:i withObject:[NSNumber numberWithBool:YES]];
        
        [content_box setContentView:current_view];
        
        currentPluginIndex = i;
        
        // TODO: resizing needs to be fixed
    } else {
        NSLog(@"Attempt to select plugin out of bounds");
    }
}


- (IBAction) incrementPlugin:(id)sender {

    int segment = [sender selectedSegment];
    
    if(segment == 0){
        [self previousPlugin:sender];
    }
    
    if(segment == 1){
        [self nextPlugin:sender];
    }
}

- (IBAction) previousPlugin:(id)sender{
    int new_index = (currentPluginIndex-1);
    if(new_index < 0){
        new_index = new_index + [plugins count];
    }
    
    self.currentPluginIndex = new_index;
}

- (IBAction) nextPlugin:(id)sender{

    self.currentPluginIndex = (currentPluginIndex+1) % [plugins count];
}

- (void) setCycling:(BOOL)value{

    if(cycleTimer != Nil){
        
        [cycleTimer invalidate];
        [cycleTimer release];
        cycleTimer = nil;
    }
    
    cycling = value;
    if(cycling){
        NSTimeInterval interval = cycleTime;
        cycleTimer = [[NSTimer scheduledTimerWithTimeInterval:interval target:self selector:@selector(nextPlugin:) userInfo:self repeats:YES] retain];
    }
}

- (void) setCycleTime:(double)time{
    
    cycleTime = time;
    self.cycling = cycling;
}


- (void)setCustomColor:(NSColor *)color{

    headerColor = color;
    [headerBox setFillColor:headerColor];
}

@end

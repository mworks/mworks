//
//  MWCalibratorWindow.m
//  MWorksCalibratorWindow
//
//  Created by Ben Kennedy on 5/3/08.
//  Copyright 2008 MIT. All rights reserved.
//

#import "MWCalibratorWindow.h"
#import "MWCalibratorWindowController.h"

// I'd love to not hard codes these in but I don't know where to find where these are defined
#define ARROW_KEY_LEFT 0x7b
#define ARROW_KEY_RIGHT 0x7c
#define ARROW_KEY_DOWN 0x7d
#define ARROW_KEY_UP 0x7e

@implementation MWCalibratorWindow

- (void)sendEvent:(NSEvent *)the_event {
	if([the_event type] == NSEventTypeKeyDown) {
		float step = 0.1;
		
		if(([the_event modifierFlags] & NSEventModifierFlagShift) == NSEventModifierFlagShift) {
			step = 1;
		}
		
		if(([the_event modifierFlags] & NSEventModifierFlagControl) == NSEventModifierFlagControl) {
			switch([the_event keyCode]) {
				case ARROW_KEY_UP:
					[controller setVOffset:[controller vOffset]+step];
					[controller updateCalibratorParams:self];
					break;				
				case ARROW_KEY_DOWN:
					[controller setVOffset:[controller vOffset]-step];
					[controller updateCalibratorParams:self];
					break;				
				case ARROW_KEY_LEFT:
					[controller setHOffset:[controller hOffset]-step];
					[controller updateCalibratorParams:self];
					break;				
				case ARROW_KEY_RIGHT:
					[controller setHOffset:[controller hOffset]+step];
					[controller updateCalibratorParams:self];
					break;
				default:
					break;
			}
		} else if(([the_event modifierFlags] & NSEventModifierFlagCommand) == NSEventModifierFlagCommand) {
			switch([the_event keyCode]) {
				case ARROW_KEY_UP:
					[controller setVGain:[controller vGain]+step];
					[controller updateCalibratorParams:self];
					break;				
				case ARROW_KEY_DOWN:
					[controller setVGain:[controller vGain]-step];
					[controller updateCalibratorParams:self];
					break;				
				case ARROW_KEY_LEFT:
					[controller setHGain:[controller hGain]-step];
					[controller updateCalibratorParams:self];
					break;				
				case ARROW_KEY_RIGHT:
					[controller setHGain:[controller hGain]+step];
					[controller updateCalibratorParams:self];
					break;
				default:
					break;
			}
		}
	}
	
	[super sendEvent:the_event];
}

@synthesize controller;

- (void)setController:(id)new_controller {
	if(![new_controller respondsToSelector:@selector(updateCalibratorParams:)] ||
	   ![new_controller respondsToSelector:@selector(hOffset)] ||
	   ![new_controller respondsToSelector:@selector(hGain)] ||
	   ![new_controller respondsToSelector:@selector(vOffset)] ||
	   ![new_controller respondsToSelector:@selector(vGain)] ||
	   ![new_controller respondsToSelector:@selector(setHOffset:)] ||
	   ![new_controller respondsToSelector:@selector(setHGain:)] ||
	   ![new_controller respondsToSelector:@selector(setVOffset:)] ||
	   ![new_controller respondsToSelector:@selector(setVGain:)]) {
		[NSException raise:NSInternalInconsistencyException
					format:@"controller for MWCalibratorWindow doesn't reqpond to required methods"];
	}
	   
	controller = new_controller;
}

@end

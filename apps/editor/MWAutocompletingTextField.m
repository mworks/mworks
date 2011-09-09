//
//  MWAutocompletingTextField.m
//  NewEditor
//
//  Created by David Cox on 11/18/07.
//  Copyright 2007 __MyCompanyName__. All rights reserved.
//

#import "MWAutocompletingTextField.h"


@implementation MWAutocompletingTextField

- (id)init{
	triggeringCompletion = 0;
	completionGuard = 0;
	return self;
}

- (NSString *)xPathForPartialTag:(NSString *)partial{
	return [NSString stringWithFormat:@"//@tag[starts-with(.,\"%@\")]", partial];
}


- (void)textDidChange:(NSNotification *)aNotification{
	[super textDidChange:aNotification];
	
	// A failed attempt at "inline" autocompletion
	// May be salvageable at some point
	
/*	//NSDictionary *d = [aNotification userInfo];
	//NSTextView *editor = [d valueForKey:@"NSFieldEditor"];
	NSTextView *editor = [self currentEditor];


	NSRange selected = [editor selectedRange];
	NSLog(@"%d characters selected", selected.length);
	
	if(!triggeringCompletion){
		
		if(completionGuard){
			completionGuard = NO;
		} else {
			triggeringCompletion =  YES;
			completionGuard = YES;
			[editor complete:self];
			triggeringCompletion = NO;
		}
	} */
}

@end

@implementation MWVariableTextField

- (NSString *)xPathForPartialTag:(NSString *)partial{ 
	return [NSString stringWithFormat:@"//variable/@tag[starts-with(.,\"%@\")]", partial];
}
@end

@implementation MWStimulusTextField

- (NSString *)xPathForPartialTag:(NSString *)partial{ 
	return [NSString stringWithFormat:@"//stimulus/@tag[starts-with(.,\"%@\")]", partial];
}
@end


@implementation MWAutocompletingComboBox

- (NSString *)xPathForPartialTag:(NSString *)partial{ 
	return [NSString stringWithFormat:@"//@tag", partial];
}
@end

@implementation MWVariableComboBox

- (NSString *)xPathForPartialTag:(NSString *)partial{ 
	return [NSString stringWithFormat:@"//variables/variable/@tag[starts-with(.,\"%@\")]", partial];
}
@end

@implementation MWLocalVariableComboBox

- (NSString *)xPathForPartialTag:(NSString *)partial{ 
	return [NSString stringWithFormat:@"//variable[contains(translate(@scope, 'ABCDEFGHIJKLMNOPQRSTUVWXYZ', 'abcdefghijklmnopqrstuvwxyz'),\"local\")]/@tag[starts-with(.,\"%@\")]", partial];
}
@end

@implementation MWGlobalVariableComboBox

- (NSString *)xPathForPartialTag:(NSString *)partial{ 
	return [NSString stringWithFormat:@"//variable[translate(@scope, 'ABCDEFGHIJKLMNOPQRSTUVWXYZ', 'abcdefghijklmnopqrstuvwxyz') = \'global\']/@tag[starts-with(.,\"%@\")]", partial];
}
@end

@implementation MWStimulusComboBox

- (NSString *)xPathForPartialTag:(NSString *)partial{ 
	return [NSString stringWithFormat:@"//stimulus/@tag[starts-with(.,\"%@\")]", partial];
}
@end


@implementation MWSoundComboBox

- (NSString *)xPathForPartialTag:(NSString *)partial{ 
	return [NSString stringWithFormat:@"//sound/@tag[starts-with(.,\"%@\")]", partial];
}
@end

@implementation MWIODeviceComboBox

- (NSString *)xPathForPartialTag:(NSString *)partial{ 
	return [NSString stringWithFormat:@"//iodevice/@tag[starts-with(.,\"%@\")]", partial];
}
@end

@implementation MWCalibratorComboBox

- (NSString *)xPathForPartialTag:(NSString *)partial{ 
	return [NSString stringWithFormat:@"//calibrator/@tag[starts-with(.,\"%@\")]", partial];
}
@end


@implementation MWTimerComboBox

- (NSString *)xPathForPartialTag:(NSString *)partial{ 
	return [NSString stringWithFormat:@"//@timer[starts-with(.,\"%@\")]", partial];
}
@end

@implementation MWTimebaseComboBox

- (NSString *)xPathForPartialTag:(NSString *)partial{ 
	return [NSString stringWithFormat:@"//action[@type=\"set_timebase\"]/@timebase[starts-with(.,\"%@\")]", partial];
}
@end

@implementation MWSelectionVariableComboBox

- (NSString *)xPathForPartialTag:(NSString *)partial{ 
	return [NSString stringWithFormat:@"//variables/selection_variable/@tag[starts-with(.,\"%@\")]", partial];
}
@end

@implementation MWSelectionComboBox
// TODO
@end



@implementation MWTaskSystemStateComboBox

- (NSString *)xPathForPartialTag:(NSString *)partial{ 
	return [NSString stringWithFormat:@"//task_system_state/@tag[starts-with(.,\"%@\")]", partial];
}
@end
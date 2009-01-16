//
//  MWAutocompletingTextField.h
//  NewEditor
//
//  Created by David Cox on 11/18/07.
//  Copyright 2007 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface MWAutocompletingTextField : NSTextField {
	BOOL triggeringCompletion;
	BOOL completionGuard;
}

- (NSString *)xPathForPartialTag:(NSString *)partial;

@end


@interface MWVariableTextField : MWAutocompletingTextField{}
@end

@interface MWStimulusTextField : MWAutocompletingTextField {}
@end

@interface MWAutocompletingComboBox : NSComboBox {}

- (NSString *)xPathForPartialTag:(NSString *)partial;

@end

@interface MWLocalVariableComboBox : MWAutocompletingComboBox {}
@end

@interface MWGlobalVariableComboBox : MWAutocompletingComboBox {}
@end

@interface MWVariableComboBox : MWAutocompletingComboBox {}
@end

@interface MWStimulusComboBox : MWAutocompletingComboBox {}
@end

@interface MWSoundComboBox : MWAutocompletingComboBox {}
@end

@interface MWIODeviceComboBox : MWAutocompletingComboBox {}
@end

@interface MWCalibratorComboBox : MWAutocompletingComboBox {}
@end

@interface MWTimerComboBox : MWAutocompletingComboBox {}
@end

@interface MWTimebaseComboBox : MWAutocompletingComboBox {}
@end

@interface MWSelectionVariableComboBox : MWAutocompletingComboBox {}
@end

@interface MWSelectionComboBox : MWAutocompletingComboBox {}
@end


@interface MWTaskSystemStateComboBox : MWAutocompletingComboBox {}
@end
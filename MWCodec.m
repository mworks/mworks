//
//  MWCodec.m
//  New Client
//
//  Created by David Cox on 3/14/08.
//  Copyright 2008 __MyCompanyName__. All rights reserved.
//

#import "MWCodec.h"
#import "MonkeyWorksCocoa/MWCocoaEventFunctor.h"
#import "MWClientInstance.h"

#define UPDATE_INTERVAL	0.1

@implementation MWCodec


- (id) initWithClientInstance: (id) _client{
	clientInstance = (MWClientInstance *)_client;
	core = [clientInstance coreClient];
	
	variable_names = [[NSMutableArray alloc] init];
	variable_codes = [[NSMutableArray alloc] init];
	variable_changed = [[NSMutableArray alloc] init];
	
	// register for codec callback
	boost::shared_ptr <mw::CocoaEventFunctor> cef;
	cef = boost::shared_ptr <mw::CocoaEventFunctor>(
						new mw::CocoaEventFunctor(self, @selector(receiveCodec:),
						MW_CODEC_CALLBACK_KEY));	
	core->registerCallback(cef, RESERVED_CODEC_CODE);
	
	
	// start updateChangedValues timer
	update_timer = [NSTimer scheduledTimerWithTimeInterval:(NSTimeInterval)UPDATE_INTERVAL 
						target:self selector:@selector(updateChangedValues) userInfo:Nil repeats:YES];
	
	return self;
}

- (void) dealloc {
	if(update_timer != Nil){
		[update_timer invalidate];
	}
	
	[super dealloc];
}


- (NSMutableArray *)variableNames{
	return variable_names;
}

// Update internally stored information given the arrival of a 
// new codec.
- (void)receiveCodec:(MWCocoaEvent *)event{
	
	@synchronized(self){
        
        [variable_names removeAllObjects];
        [variable_changed removeAllObjects];
        
        // get variable names
        vector<string> variable_names_stl = core->getVariableNames();
        vector<string>::iterator i;
        for(i = variable_names_stl.begin(); i != variable_names_stl.end(); i++){
            string variable_name_stl = *i;
            if(variable_name_stl.empty()){
                continue;
            }
            NSString *variable_name = [NSString stringWithCString:variable_name_stl.c_str()];
            [variable_names addObject:variable_name];
            [variable_codes addObject:[NSNumber numberWithInteger:core->getCode(variable_name_stl)]];
            [self willChangeValueForKey:variable_name];
            [self didChangeValueForKey:variable_name];
            
            [variable_changed addObject:[NSNumber numberWithBool:NO]];
        }
        
        [clientInstance willChangeValueForKey:@"variables"];
        [clientInstance didChangeValueForKey:@"variables"];
        [self willChangeValueForKey:@"variableNames"];
        [self didChangeValueForKey:@"variableNames"];
    }
}


- (NSMutableArray *)mutableArrayValueForKey:(NSString *)key {
	@synchronized(self){
        return [self variableNames];
    }	
	return 0;
}

- (int)countOfVariableNames {
	@synchronized(self){
        return [variable_names count];
    }
	return 0;
}

- (id) objectInVariableNamesAtIndex: (int)i {
	@synchronized(self){
        return [variable_names objectAtIndex:i];
    }
	return 0;
}

// KVC compliance (keys are variable names)
- (id) valueForKey: (NSString *)_key{
	
    @synchronized(self){
        try {
            if([_key isEqualToString:@"variableNames"]){
                NSLog(@"doing it...");
                return [self variableNames];
            }
            
            NSString *key = [NSString stringWithString:_key];
            
            if([key length] == 0){
                return Nil;
            }
            
            // transform "_variable" to "#variable" if necessary
            
            const char *ckey = [key cStringUsingEncoding:NSASCIIStringEncoding];
            if(ckey == NULL){
                return Nil;
            }
            string skey(ckey);
            if(skey[0] == '_'){
                skey[0] = '#';
                key = [NSString stringWithCString:skey.c_str()];
            }
            
            // ask core client for value
            shared_ptr<mw::Variable> variable = core->getVariable([key cStringUsingEncoding:NSASCIIStringEncoding]);
            
            if(variable == NULL){
                return [self valueForUndefinedKey:key];
            }
            
            mw::Data value = variable->getValue();
            
            string value_string = value.toString();
            
            if(value_string.empty()){
                return Nil;
            } else {
                // format value?
                return [NSString stringWithCString:value_string.c_str()];
            }
        } catch (std::exception& e){
            return Nil;
        }
    }
	
	return Nil;
}

- (id) valueForUndefinedKey: (NSString *)key {
	return Nil;
}


- (void) setValue: (id)val forKey: (NSString *)key{
	
    // lookup code
	int code = core->getCode([key cStringUsingEncoding:NSASCIIStringEncoding]);
	
	// format value appropriately
	mw::Data setval;
	if(1 || [val isKindOfClass:[NSNumber class]]){
		setval.setFloat([val floatValue]);
		
		NSLog(@"value is %g", val);
		
	} else if([val isKindOfClass:[NSString class]]){
		NSScanner *scanner = [[NSScanner alloc] initWithString:val];
		double possibleDoubleValue;

		if([scanner scanDouble:&possibleDoubleValue]) {
			if([scanner isAtEnd]) {
				setval.setFloat(possibleDoubleValue);
			}
		}
		
		NSLog(@"value is: %g", possibleDoubleValue);
	}

	// tell core client to set it	
	NSLog(@"Set %@ (code = %d) to %@", key, code, val);
	
    @synchronized(self){
        [self willChangeValueForKey:key];
        core->updateValue(code, setval);
        [self didChangeValueForKey:key];
    }
	
	NSLog(@"%@ now equals %@", key, [self valueForKey:key]);
}



// DDC, 8/20/2008:  DEPRECATED:  KVC events are driven by a timer
//					Keeping this around for now in case we decide to use 
//					a hybrid strategy later (e.g. throttled callbacks)
-(void)handleCallbackEventInMainThread:(MWCocoaEvent *)event {

    [self performSelectorOnMainThread:
			@selector(handleCallbackEvent:) 
							   withObject: event waitUntilDone: NO];
}

// DDC, 8/20/2008:  DEPRECATED:  KVC events are driven by a timer
- (void)handleCallbackEvent:(MWCocoaEvent *)event{
	// set changed flag for value
	
	int code = [event code];
	NSNumber *code_number = [NSNumber numberWithInteger:code];
	
    @synchronized(self){
        int index = [variable_codes indexOfObject:code_number];
        [variable_changed replaceObjectAtIndex:index withObject:[NSNumber numberWithBool:YES]];
    }
}

- (void)updateChangedValues{
	BOOL any = NO;

	@synchronized(self){
        [clientInstance willChangeValueForKey:@"variables"];
        [clientInstance didChangeValueForKey:@"variables"];

        // find changed values, and send will/did change, then unset flag
        for(int i = 0; i < [variable_changed count]; i++){
            if([[variable_changed objectAtIndex:i] boolValue]){
                NSString *variable_name = [variable_names objectAtIndex:i];
                
                [self willChangeValueForKey:variable_name];
                [self didChangeValueForKey:variable_name];
                
                
                NSString *keypath = [NSString stringWithString:@"variables."];
                keypath = [keypath stringByAppendingString:variable_name];
                
                
                
                any = YES;
            }
        }
    }
}

@end


//
//  MWCodec.m
//  New Client
//
//  Created by David Cox on 3/14/08.
//  Copyright 2008 Harvard University. All rights reserved.
//

#import "MWCodec.h"
#import "MWCocoaEventFunctor.h"

#define MW_CODEC_CALLBACK_KEY "mw_codec_callback_key"
#define UPDATE_INTERVAL	0.1

@implementation MWCodec


- (id) initWithClientInstance: (id<MWClientProtocol>) _client{
	clientInstance = _client;
	
    core = [clientInstance coreClient];
  
	variable_names = [[NSMutableArray alloc] init];
	variable_codes = [[NSMutableArray alloc] init];
	variable_changed = [[NSMutableArray alloc] init];
	
	// register for codec callback
	[clientInstance registerEventCallbackWithReceiver:self 
                                             selector:@selector(receiveCodec:)
                                          callbackKey:MW_CODEC_CALLBACK_KEY
                                      forVariableCode:RESERVED_CODEC_CODE
                                         onMainThread:NO];
  
	// start updateChangedValues timer
	update_timer = [NSTimer scheduledTimerWithTimeInterval:(NSTimeInterval)UPDATE_INTERVAL 
                                                  target:self 
                                                selector:@selector(updateChangedValues) 
                                                userInfo:Nil 
                                                 repeats:YES];
	
	return self;
}

- (void) dealloc {
	if(update_timer != Nil){
		[update_timer invalidate];
	}
	
}


- (NSMutableArray *)variableNames{
	return variable_names;
}


// Update internally stored information given the arrival of a 
// new codec.
- (void)receiveCodec:(MWCocoaEvent *)event{
	
	@synchronized(clientInstance){
  
    //NSLog(@"Entered receivedCodec:");
    [variable_names removeAllObjects];
    [variable_changed removeAllObjects];
    
    // get variable names
    vector<string> variable_names_vector = core->getVariableTagNames();
    
    vector<string>::iterator i;
    for(i = variable_names_vector.begin(); i != variable_names_vector.end(); i++){
        string variable_name_string = *i;
        if(variable_name_string.empty()){
            continue;
        }
        NSString *variable_name = [NSString stringWithCString:variable_name_string.c_str() encoding:NSASCIIStringEncoding];
        
        int code = core->lookupCodeForTag(variable_name_string);
      
        [variable_names addObject:variable_name];
        [variable_codes addObject:[NSNumber numberWithInteger:code]];
    
        [self willChangeValueForKey:variable_name];
        [self didChangeValueForKey:variable_name];
        
        [variable_changed addObject:[NSNumber numberWithBool:NO]];
    }
    
    //[clientInstance willChangeValueForKey:@"variables"];
    //[clientInstance didChangeValueForKey:@"variables"];
    [self willChangeValueForKey:@"variableNames"];
    [self didChangeValueForKey:@"variableNames"];
  }
  //NSLog(@"Leaving receivedCodec:");
}


- (NSMutableArray *)mutableArrayValueForKey:(NSString *)key {
	@synchronized(clientInstance){
        return [self variableNames];
    }	
	return 0;
}

- (int)countOfVariableNames {
	@synchronized(clientInstance){
        return [variable_names count];
    }
	return 0;
}

- (id) objectInVariableNamesAtIndex: (int)i {
	@synchronized(clientInstance){
        return [variable_names objectAtIndex:i];
    }
	return 0;
}

// KVC compliance (keys are variable names)
- (id) valueForKey: (NSString *)_key{
	
    @synchronized(clientInstance){
        //NSLog(@"Entered valueForKey:");
        try {
            if([_key isEqualToString:@"variableNames"]){
                //NSLog(@"doing it...");
                //NSLog(@"Leaving valueForKey: (variableNames)");
                return [self variableNames];
            }
            
            NSString *key = [NSString stringWithString:_key];
            
            if([key length] == 0){
                //NSLog(@"Leaving valueForKey: (zero length key)");
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
                key = [NSString stringWithCString:skey.c_str() encoding:NSASCIIStringEncoding];
            }
            
            mw::Datum value = [self valueForVariable:key];
            
            if (value.isUndefined()) {
                //NSLog(@"Leaving valueForKey: (NULL variable)");
                return [self valueForUndefinedKey:key];
            }
            
            string value_string = value.toString();
          
            if(value_string.empty()){
              //NSLog(@"Leaving valueForKey: (empty string)");  
              return Nil;
            } else {
                // format value?
                //NSLog(@"Leaving valueForKey:");
                return [NSString stringWithCString:value_string.c_str() encoding:NSASCIIStringEncoding];
            }
        } catch (std::exception& e){
            //NSLog(@"Leaving valueForKey: (with c++ exception)");
            return Nil;
        }
    }
  
  //NSLog(@"Leaving valueForKey: (fall through)");
	
	return Nil;
}

- (id) valueForUndefinedKey: (NSString *)key {
	return Nil;
}


- (void) setValue: (id)val forKey: (NSString *)key{
	// format value appropriately
	mw::Datum setval;
	if ([val isKindOfClass:[NSNumber class]]) {
		setval.setFloat([val floatValue]);
		
		//NSLog(@"value is %g", [val floatValue]);
		
	} else if([val isKindOfClass:[NSString class]]) {
		NSScanner *scanner = [[NSScanner alloc] initWithString:val];
		double possibleDoubleValue;

		if ([scanner scanDouble:&possibleDoubleValue] && [scanner isAtEnd]) {
            setval.setFloat(possibleDoubleValue);
            //NSLog(@"value is: %g", possibleDoubleValue);
        } else {
            setval.setString([val cStringUsingEncoding:NSASCIIStringEncoding]);				
            //NSLog(@"value is: %@", val);
        }
        
	}
    
    [self setValue:setval forVariable:key];
}


- (void)updateChangedValues{
  BOOL any = NO;

  @synchronized(clientInstance){
    //NSLog(@"Entered updateChangedValues:");
    //[clientInstance willChangeValueForKey:@"variables"];
    //[clientInstance didChangeValueForKey:@"variables"];

    // send will/did change
    for(int i = 0; i < [variable_changed count]; i++){
        
      NSString *variable_name = [variable_names objectAtIndex:i];
      
      [self willChangeValueForKey:variable_name];
      [self didChangeValueForKey:variable_name];
      
      NSString *keypath = @"variables.";
      keypath = [keypath stringByAppendingString:variable_name];
      
      any = YES;
    }
  }
  //NSLog(@"Leaving updateChangedValues");
}


- (mw::Datum)valueForVariable:(NSString *)key {
    @synchronized (clientInstance) {
        shared_ptr<mw::Variable> variable = core->getVariable([key cStringUsingEncoding:NSASCIIStringEncoding]);
        
        if (!variable) {
            return mw::Datum();
        }
        
        return variable->getValue();
    }
}


- (void)setValue:(const mw::Datum &)setval forVariable:(NSString *)key {
    @synchronized (clientInstance) {
        int code = core->lookupCodeForTag([key cStringUsingEncoding:NSASCIIStringEncoding]);
        if (code >= 0) {
            [self willChangeValueForKey:key];
            core->updateValue(code, setval);
            [self didChangeValueForKey:key];
        }
    }
}


- (NSString *)descriptionForVariable:(NSString *)name {
    @synchronized (clientInstance) {
        if (auto variable = core->getVariable(name.UTF8String)) {
            return @(variable->getProperties().getDescription().c_str());
        }
        return @"";
    }
}


@end

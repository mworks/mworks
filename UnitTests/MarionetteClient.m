
/**
 * Marionette.m
 *
 * History:
 * bkennedy on 07/05/07 - Created.
 *
 * Copyright MIT 2007.  All rights reserved.
 */

#import "MarionetteClient.h"
#import "MarionetteMessage.h"
#import "MarionetteEvent.h"
#import "MWCocoaEventFunctor.h"
#import "MarionetteGlobals.h"
#import "MarionetteMessage.h"
#import "MarionetteEvent.h"
#import "MonkeyWorksCore/EventFactory.h"
#import "MonkeyWorksCore/XMLParser.h"
#import "MonkeyWorksCore/ExpressionVariable.h"

Data _parseDataValue(xmlNode *node);
string _attributeForName(xmlNode *node, string name);
vector<xmlNode *> _getChildren(xmlNode *node);
vector<xmlNode *> _getChildren(xmlNode *node, string tag);
string _getContent(xmlNode *node);
Data _getNumber(const string &expression, const GenericDataType type);


#define MARIONETTE_KEY "marionette key"
#define MARIONETTE_TEST_DATA_FILE_PREFIX "MARIONETTE."

@interface MarionetteClient (PrivateMethods)
- (void)runExperiment:(NSString *)experiement_path;
- (void)marionetteAssert:(BOOL)test withMessage:(NSString *)assert_message;
- (void)marionetteAssert:(NSString *)assert_message;
- (void) checkMessageStructure:(Data *)event_data;
- (BOOL) checkErrorMessageForKnownErrors:(NSString *)message;
@end

@implementation MarionetteClient

- (id) init {
	self = [super init];
	if (self != nil) {
		client = new Client();
		
		boost::shared_ptr <CocoaEventFunctor> cef = boost::shared_ptr <CocoaEventFunctor>(new CocoaEventFunctor(self,
																												   @selector(eventReceived:), 
																												   MARIONETTE_KEY));
		client->registerCallback(cef);
		client->startEventListener();
		
		asserted=NO;
		
		state_system_running = NO;
		experiment_ended = NO;
		sent_run_event = NO;
		sent_experiment = NO;
		experiment_loaded = NO;
		sent_close_experiment = NO;
		sent_open_data_file = NO;
		sent_close_data_file = NO;
		data_file_open = NO;
		
		permitted_error_messages = [[NSArray alloc] init];
		expected_events = [[NSMutableArray alloc] init];
		expected_messages = [[NSMutableArray alloc] init];
		
	}
	return self;
}



- (void)dealloc {
	[expected_messages release];
	[expected_events release];
	[permitted_error_messages release];
	delete client;
	[super dealloc];
}

- (void)awakeFromNib {
	// get the list of permitted error messages (if any) and expected messages
	if([[[NSProcessInfo processInfo] arguments] count] > 2) {
		
		NSString *message_file = [[[NSProcessInfo processInfo] arguments] objectAtIndex:2];
		
		
		
		
		
		LIBXML_TEST_VERSION
		
		xmlParserCtxt *context = xmlNewParserCtxt();	
		if(context == NULL){
			NSLog(@"couldn't create XML context");
			exit(1);
		}
		
		xmlSetGenericErrorFunc(context, &error_func);
		
		// parse the file and get the DOM 
		xmlDoc *xml_doc = xmlCtxtReadFile(context, [message_file cStringUsingEncoding:NSASCIIStringEncoding], NULL, 0);
		
		
		// get the xsl doco out of the main bundle
		NSBundle *main_bundle = [NSBundle mainBundle];
		NSString *simplification_path = [main_bundle pathForResource:@"MarionetteParserTransformation" ofType:@"xsl"];
		xsltStylesheet *simplification_transform = xsltParseStylesheetFile((const xmlChar *)([simplification_path cStringUsingEncoding:NSASCIIStringEncoding]));
		xmlDoc *simplified = xsltApplyStylesheet(simplification_transform, xml_doc, NULL);
		
		xmlNode *root_element = xmlDocGetRootElement(simplified);
		
		//xmlDocDump(stderr, xml_doc);
		//xmlDocDump(stderr, simplified);
		
		xmlNode *child = root_element->children;
		while(child != NULL){
			string name((const char *)(child->name));
			NSLog(@"child_node: %s", name.c_str());
			
			if(name=="event") {
				// Parse the attributes
				_xmlAttr *att = child->properties;
				
				string variable;
				BOOL continue_processing = YES;
				while( att != NULL){
					string attribute_name((const char *)(att->name));
					if(attribute_name == "variable"){
						variable = (const char *)(att->children->content);
					}			
					
					if(attribute_name == "continue"){
						continue_processing = _getNumber((const char *)(att->children->content),
														 M_BOOLEAN).getBool();
					}			
					
					att = att->next;
				}
				
				
				xmlNode *subchild = child->children;
				
				while(subchild != NULL) {
					string n((const char *)subchild->name);
					if(n == "value") {
						break;
					}
					subchild = subchild->next;
				}
				
				Data data(_parseDataValue(subchild));
				
				MarionetteEvent *me = [MarionetteEvent eventWithVariableName:[NSString stringWithCString:variable.c_str() 
																								encoding:NSASCIIStringEncoding] 
																	 andData:&data 
													 andProcessDataAfterward:continue_processing];
				
				[self.expectedEvents addObject:me];
			}
			child = child->next;
		}		
		
		NSXMLDocument *info_xml = [[[NSXMLDocument alloc] initWithData:[NSData dataWithContentsOfFile:message_file]
															   options:0
																 error:nil] autorelease];
		
		if(info_xml == nil) {
			NSLog(@"xml_file: %@ couldn't be opened", message_file);
			exit(1);
		}
		
		{
			// permitted error messages
			NSMutableArray *temp_pe_array = [NSMutableArray array];
			NSArray *permitted_error_nodes = [info_xml nodesForXPath:@"./marionette_info/permitted_error_messages/message"
															   error:nil];
			
			NSEnumerator *pe_node_enum = [permitted_error_nodes objectEnumerator];
			NSXMLElement *pe_node = nil;
			
			while(pe_node = [pe_node_enum nextObject]) {
				MarionetteMessage *message = [MarionetteMessage messageWithXMLNode:pe_node];				
				if(message == nil) {
					NSLog(@"MarionetteMessage couldn't be created");
					exit(1);
				}
				
				[temp_pe_array addObject:message];
			}
			
			self.permittedErrorMessages = temp_pe_array;
		}
		{
			NSArray *expected_message_nodes = [info_xml nodesForXPath:@"./marionette_info/expected_messages/message"
																error:nil];
			
			NSEnumerator *em_node_enum = [expected_message_nodes objectEnumerator];
			NSXMLElement *em_node = nil;
			
			while(em_node = [em_node_enum nextObject]) {
				MarionetteMessage *message = [MarionetteMessage messageWithXMLNode:em_node];				
				if(message == nil) {
					NSLog(@"MarionetteMessage couldn't be created");
					exit(1);
				}
				
				[self.expectedMessages addObject:message];
			}
		}
	}
	
	while(!client->isConnected()) {
		// give server time to fully initialize before trying to connect
		[NSThread sleepUntilDate:[NSDate dateWithTimeIntervalSinceNow:5]];
		client->connectToServer(DEFAULT_HOST_IP, DEFAULT_LOW_PORT);			
	}
	
	[NSTimer scheduledTimerWithTimeInterval:2 target:self selector:@selector(waitForExperimentToEnd:) userInfo:nil repeats:YES];
}

@synthesize permittedErrorMessages=permitted_error_messages;
@synthesize expectedMessages=expected_messages;
@synthesize expectedEvents=expected_events;
@synthesize dataFileOpen=data_file_open;
@synthesize sentOpenDataFile=sent_open_data_file;
@synthesize sentCloseDataFile=sent_close_data_file;
@synthesize sentRunEvent=sent_run_event;
@synthesize stateSystemRunning=state_system_running;
@synthesize asserted=asserted;
@synthesize experimentEnded=experiment_ended;
@synthesize experimentLoaded=experiment_loaded;
@synthesize sentExperiment=sent_experiment;
@synthesize sentCloseExperiment=sent_close_experiment;

- (void)waitForExperimentToEnd:(NSTimer *)the_timer {
	if(self.experimentEnded || self.asserted) {
		if(!self.asserted && ([self.expectedMessages count] > 0 || [self.expectedEvents count] > 0)) {
			if([self.expectedMessages count] > 0) {
				[self marionetteAssert:[NSString stringWithFormat:@"not all required messages were recevied in the proper order.  Next expected: %@", [[self.expectedMessages objectAtIndex:0] message]]]; 
			}
			
			if([self.expectedEvents count] > 0) {
				[self marionetteAssert:@"not all required events were received"]; 
			}
		}
		
		if(!self.asserted) {
			[self marionetteAssert:!self.dataFileOpen
					   withMessage:@"Data file is open when it should be closed"];
			[self marionetteAssert:!client->isConnected()
					   withMessage:@"client should no longer be connected"];
		}
		
		exit(self.asserted);
	}
}

// private methods
- (void)eventReceived:(MWCocoaEvent *)event {
	int code = [event code];
	
	if([self.expectedEvents count] > 0) {
		
		// 'retain' then 'autorelease' because the event's reference count reaches zero when removeObjectAtIndex:0 is called later
		MarionetteEvent *next_expected_event = [[[self.expectedEvents objectAtIndex:0] retain] autorelease];
		int next_expected_event_code = client->getCode([[next_expected_event variable] cStringUsingEncoding:NSASCIIStringEncoding]);
		if(code == next_expected_event_code && code != -1) {
			if([next_expected_event matches:[event data]]) {
				[self.expectedEvents removeObjectAtIndex:0];
				if(![next_expected_event continueProcessing]) {
					return;
				}
			}
		}
	}	
	
	int message_codec_code = client->getCode(ANNOUNCE_MESSAGE_VAR_TAGNAME);
	int system_codec_code = client->getCode(SYSTEM_VAR_TAGNAME);
	int assert_codec_code = client->getCode(ANNOUNCE_ASSERTION_TAGNAME);
	int taskMode_codec_code = client->getCode(TASK_MODE_TAGNAME);
	
	if (code == RESERVED_CODEC_CODE) {
		client->unregisterCallbacks(MARIONETTE_KEY);
		
		boost::shared_ptr <CocoaEventFunctor> cef = boost::shared_ptr <CocoaEventFunctor>(new CocoaEventFunctor(self,
																												   @selector(eventReceived:), 
																												   MARIONETTE_KEY));
		client->registerCallback(cef);
	} else if (code == system_codec_code && system_codec_code > RESERVED_CODEC_CODE) {
		Data event_data(*[event data]);
		
		
		
		if(!event_data.isDictionary()) {
			// this is crap....TODO: whys are we even here?
			//			[self marionetteAssert:!event_data.getElement(M_CURRENT_PROTOCOL).isUndefined()
			//					   withMessage:@"crappy part is even crappier"]; 
			//			[self marionetteAssert:!event_data.getElement(M_EXPERIMENT_NAME).isUndefined()
			//					   withMessage:@"crappy part is even crappier"]; 
			//			[self marionetteAssert:!event_data.getElement(M_PROTOCOLS).isUndefined()
			//					   withMessage:@"crappy part is even crappier"]; 
		} else {		
			Data sys_event_type(event_data.getElement(M_SYSTEM_PAYLOAD_TYPE));
			if(!sys_event_type.isUndefined()) {
				[self marionetteAssert:sys_event_type.isInteger()
						   withMessage:@"system event type isn't an integer"]; 
				// The F swich statement ... it's gotta be somewhere
				switch((SystemPayloadType)sys_event_type.getInteger()) {
					case M_EXPERIMENT_STATE:
					{
						Data state(event_data.getElement(M_SYSTEM_PAYLOAD));
						[self marionetteAssert:state.isDictionary()
								   withMessage:@"system state event is not a dictionary"]; 
						
						[self marionetteAssert:state.getElement(M_LOADED).isInteger()
								   withMessage:@"system state event 'loaded' element is not integer"]; 
						
						if(state.getElement(M_LOADED).getBool()) {
							[self marionetteAssert:state.getElement(M_PAUSED).isInteger()
									   withMessage:@"system state event 'paused' element is not integer"]; 
							[self marionetteAssert:state.getElement(M_RUNNING).isInteger()
									   withMessage:@"system state event 'running' element is not integer"]; 
							[self marionetteAssert:state.getElement(M_EXPERIMENT_NAME).isString()
									   withMessage:@"system state event 'experiment name' element is not string"]; 
							[self marionetteAssert:state.getElement(M_EXPERIMENT_PATH).isString()
									   withMessage:@"system state event 'experiment path' element is not string"]; 
							
							if(!state.getElement(M_RUNNING).getBool() && !self.experimentLoaded && !self.stateSystemRunning) {
								self.experimentLoaded=YES;							
								[self marionetteAssert:self.sentExperiment
										   withMessage:@"server reports that experiment is loaded, but client hasn't sent yet"]; 
								
								[self marionetteAssert:!self.sentOpenDataFile
										   withMessage:@"already sent open data file command"];
								[self marionetteAssert:!self.dataFileOpen
										   withMessage:@"data file already open"];
								
								std::string data_file(MARIONETTE_TEST_DATA_FILE_PREFIX);
								data_file.append([[[[[[NSProcessInfo processInfo] arguments] objectAtIndex:1] lastPathComponent] stringByDeletingPathExtension] cStringUsingEncoding:NSASCIIStringEncoding]);
								
								client->sendOpenDataFileEvent(data_file,
															  [[NSNumber numberWithBool:YES] intValue]);
								self.sentOpenDataFile=YES;							
							}
						} else {
							if(!self.sentExperiment) {							
								[self marionetteAssert:!self.sentExperiment
										   withMessage:@"received trying to send an experiment more than once"]; 
								client->sendExperiment([[[[NSProcessInfo processInfo] arguments] objectAtIndex:1] cStringUsingEncoding:NSASCIIStringEncoding]);
								self.sentExperiment = YES;
							}
							
						}
					}
						break;
					case M_DATA_FILE_OPENED:
						[self marionetteAssert:self.sentOpenDataFile
								   withMessage:@"received M_OPEN_DATA_FILE without attempting to open"]; 
						
						[self marionetteAssert:!self.dataFileOpen
								   withMessage:@"received M_OPEN_DATA_FILE when data file is already opened"]; 
						self.dataFileOpen = YES;
						
						[self marionetteAssert:self.experimentLoaded
								   withMessage:@"trying to send run command without loading experiment first"]; 
						if(!self.sentRunEvent) {
							client->sendRunEvent();
							self.sentRunEvent = YES;
						}
						
						break;
					case M_CLOSE_EXPERIMENT:
						[self marionetteAssert:self.sentCloseExperiment
								   withMessage:@"received M_CLOSE_EXPERIMENT without attempting to close"]; 
						self.experimentEnded = YES;
						
						client->disconnectClient();
						break;
					case M_DATA_FILE_CLOSED:
						//					[self marionetteAssert:self.sentCloseDataFile && self.dataFileOpen
						//							   withMessage:@"received M_CLOSE_DATA_FILE without attempting to close"]; 
						if(self.sentCloseDataFile) {
							[self marionetteAssert:self.dataFileOpen
									   withMessage:@"received M_CLOSE_DATA_FILE when data file isn't opened"]; 
							self.dataFileOpen = NO;
							
							client->sendCloseExperimentEvent([[[[NSProcessInfo processInfo] arguments] objectAtIndex:1] cStringUsingEncoding:NSASCIIStringEncoding]);
							self.sentCloseExperiment = YES;
						}
						break;
					case M_EXPERIMENT_PACKAGE:
					case M_PROTOCOL_PACKAGE:
					case M_DATAFILE_PACKAGE:
					case M_PROTOCOL_SELECTION:
					case M_START_EXPERIMENT:
					case M_STOP_EXPERIMENT:
					case M_PAUSE_EXPERIMENT:
					case M_SAVE_VARIABLES:
					case M_LOAD_VARIABLES:
					case M_OPEN_DATA_FILE:
					case M_CLOSE_DATA_FILE:
					case M_CLIENT_CONNECTED_TO_SERVER:
					case M_CLIENT_DISCONNECTED_FROM_SERVER:
					case M_SERVER_CONNECTED_CLIENT:
					case M_SERVER_DISCONNECTED_CLIENT:
					case M_USER_DEFINED:
						break;
					default:
						[self marionetteAssert:[NSString stringWithFormat:@"illeagal #system type: %d", sys_event_type.getInteger()]]; 
						break;
				}
			}
		}
	} else if (code == assert_codec_code && assert_codec_code > RESERVED_CODEC_CODE) {
		Data event_data(*[event data]);
		if(event_data.isString()) {
			NSString *assert_message = [NSString stringWithCString:event_data.getString() 
														  encoding:NSASCIIStringEncoding];
			if([assert_message length] > 0) {
				[self marionetteAssert:[@"Recieved assert: " stringByAppendingString:assert_message]];
			}
		} else {
			// if it's not a string it shoud be equal to zero
			[self marionetteAssert: event_data.isInteger()
					   withMessage:@"init assert is not an int"]; 
			[self marionetteAssert: event_data.getInteger() == 0
					   withMessage:@"init assert is not equal to 0"]; 
		}
	} else if(code == taskMode_codec_code && taskMode_codec_code > RESERVED_CODEC_CODE) {
		Data event_data(*[event data]);
		[self marionetteAssert: event_data.isInteger()
				   withMessage:@"task_mode is not an int"]; 
		switch(event_data.getInteger()) {
			case STOPPING:
				[self marionetteAssert:self.stateSystemRunning
						   withMessage:@"stateSystemRunning is false"]; 
				break;
			case IDLE:
				if (self.stateSystemRunning) {
					[self marionetteAssert:self.dataFileOpen
							   withMessage:@"data file should be open"]; 
					
					std::string data_file(MARIONETTE_TEST_DATA_FILE_PREFIX);
					data_file.append([[[[[[NSProcessInfo processInfo] arguments] objectAtIndex:1] lastPathComponent] stringByDeletingPathExtension] cStringUsingEncoding:NSASCIIStringEncoding]);
					
					client->sendCloseDataFileEvent(data_file);
					self.sentCloseDataFile = YES;
					self.stateSystemRunning = NO;
					
				}
				break;
			case RUNNING:
				self.stateSystemRunning = YES;								
				break;
			default:
				[self marionetteAssert:@"illegal task_mode value"]; 
				break;
		}
	} else if(code == message_codec_code && message_codec_code > RESERVED_CODEC_CODE) {
		Data event_data(*[event data]);
		if(event_data.isInteger()) {
			// the message variable defaults to an integer equal to zero
			[self marionetteAssert:event_data.getInteger() == 0
					   withMessage:@"message is an integer?"]; 			
		} else {
			[self checkMessageStructure:&event_data];
			const char *c_message = event_data.getElement(M_MESSAGE).getString();
			NSString *message = [NSString stringWithCString:c_message
												   encoding:NSASCIIStringEncoding];
			
			//NSLog(message);
			const Data msgType(event_data.getElement(M_MESSAGE_TYPE));
			const MessageType type = (MessageType)msgType.getInteger();
			[self marionetteAssert:type >= M_GENERIC_MESSAGE && type < M_MAX_MESSAGE_TYPE
					   withMessage:@"mesasge type is not valid"]; 
			
			
			// the 'check_error_message' is so you can have expected error messages, but you don't
			// have to put them in the 'permitted error messages' portion
			BOOL check_error_message = YES;
			
			if([self.expectedMessages count] > 0) {
				MarionetteMessage *me = [self.expectedMessages objectAtIndex:0];
				if([me matches:message]) {
					[self.expectedMessages removeObjectAtIndex:0];
					
					// I could have just returned here, but we might want to do further 
					// processing upon exiting the event checking
					check_error_message = NO;
				}
			}
			
			if(type == M_ERROR_MESSAGE && check_error_message) {
				[self marionetteAssert:[self checkErrorMessageForKnownErrors:message]
						   withMessage:message]; 
			}
			
		}		
	}
}

- (void)marionetteAssert:(BOOL)test withMessage:(NSString *)assert_message {
	if(!test) {
		self.asserted = YES;
		NSLog(@"%@", assert_message);
	}
}

- (void)marionetteAssert:(NSString *)assert_message {
	[self marionetteAssert:NO withMessage:assert_message];
}


- (void) checkMessageStructure:(Data *)event_data {
	Data payload(*event_data);
	
	[self marionetteAssert:payload.isDictionary()
			   withMessage:@"messagePayload is not a dictionary"];                             
	[self marionetteAssert:payload.getElement(M_MESSAGE).isString()
			   withMessage:@"message is not a string"];                                        
	
	
	const Data msgType(payload.getElement(M_MESSAGE_TYPE));                        
	[self marionetteAssert:msgType.isInteger()
			   withMessage:@"mesasge type is not an int"];                                     
	const MessageType type = (MessageType)msgType.getInteger();                   
	
	[self marionetteAssert:type >= M_GENERIC_MESSAGE && type < M_MAX_MESSAGE_TYPE
			   withMessage:@"mesasge type is not valid"];                                      
	
	[self marionetteAssert:type != M_FATAL_ERROR_MESSAGE
			   withMessage:[NSString stringWithCString:payload.getElement(M_MESSAGE).getString()
											  encoding:NSASCIIStringEncoding]];
}

- (BOOL) checkErrorMessageForKnownErrors:(NSString *)error_message {
	MarionetteMessage *pem = nil;
	NSEnumerator *pem_enum = [self.permittedErrorMessages objectEnumerator];
	
	while(pem = [pem_enum nextObject]) {
		if([pem matches:error_message]) {
			return YES;
		}	
	}
	
	return NO;
	
}

@end


Data _parseDataValue(xmlNode *node){	
	if(node == 0) {
		return Data();
	}	
	
	
	// if node has "value" field, just use that
	// call getValue() to convert the string into a datum
	string value_field_contents = _attributeForName(node, "value");
	string type_string = _attributeForName(node, "type");
	
	GenericDataType type = M_UNDEFINED;
	
	// TODO standardize this
	if(type_string == "integer"){
		type = M_INTEGER;
	} else if(type_string == "float"){
		type = M_FLOAT;
	} else if(type_string == "boolean"){
		type = M_BOOLEAN;
	} else if (type_string == "string"){
		type = M_STRING;
	}
	
	if(value_field_contents.empty() == false){
		
		if(type == M_STRING){
			return Data(value_field_contents);
		} else if(type != M_UNDEFINED){
			return Data(value_field_contents);
			//return registry->getNumber(value_field_contents, type);
		} else {
			return Data(value_field_contents);
		}
	}
	
	// otherwise, look into the contents of the node
	vector<xmlNode *> children = _getChildren(node);
	
	if(children.size() == 0){
		if(type != M_UNDEFINED){
			return _getNumber(_getContent(node), type);
		} else {
			return Data(_getContent(node));
		}
	}
	
	if(children.size() > 1){
		NSLog(@"illegal content in expected events");
		exit(1);
	}
	
	xmlNode *child = children[0];
	
	string child_name((const char *)child->name);
	
	// if there is a <dictionary> node inside
	if(child_name == "dictionary"){
		
		vector<xmlNode *> elements = _getChildren(child, "dictionary_element");
		vector<xmlNode *>::iterator el = elements.begin();
		
		vector<Data> keys;
		vector<Data> values;
		
		for ( el = elements.begin(); el != elements.end(); el++ ){
			vector<xmlNode *> key_nodes = _getChildren(*el, "key");
			if(key_nodes.size() != 1){
				NSLog(@"illegal content in expected events");
				exit(1);
			}
			
			xmlNode *key_node = key_nodes[0];
			
			vector<xmlNode *> value_nodes = _getChildren(*el, "value");
			if(value_nodes.size() != 1){
				NSLog(@"illegal content in expected events");
				exit(1);
			}
			
			xmlNode *value_node = value_nodes[0];
			
			Data key_data = _parseDataValue(key_node);
			keys.push_back(key_data);
			
			Data value_data = _parseDataValue(value_node);
			values.push_back(value_data);
		}
		
		Data dictionary(M_DICTIONARY, (int)keys.size());
		
		vector<Data>::iterator k = keys.begin();
		vector<Data>::iterator v = values.begin();
		while(k != keys.end() && v != values.end()){
			dictionary.addElement(*k, *v);
			k++; v++;
		}
		
		return dictionary;
		
		// if there is a <list> node inside
	} else if(child_name == "list"){
		
		vector<xmlNode *> elements = _getChildren(child, "list_element");
		
		vector<Data> values;
		vector<xmlNode *>::iterator el = elements.begin();
		
		while(el != elements.end()){
			
			Data value_data = _parseDataValue(*el);
			values.push_back(value_data);
			el++;
		}
		
		Data list(M_LIST, (int)values.size());
		
		vector<Data>::iterator v = values.begin();
		while(v != values.end()){
			list.addElement(*v);
			v++;
		}
		
		return list;
	} else {
		NSLog(@"illegal content in expected events");
		exit(1);
	}
}

string _attributeForName(xmlNode *node, string name){
	// Parse the attributes
	_xmlAttr *att = node->properties;
	
	while( att != NULL){
		
		if(name == (const char *)(att->name)){
			return string((const char *)(att->children->content));
		}			
		att = att->next;
	}
	
	return string();
}

vector<xmlNode *> _getChildren(xmlNode *node){
	string empty("");
	return _getChildren(node, empty);
}

vector<xmlNode *> _getChildren(xmlNode *node, string tag){
	
	vector<xmlNode *> children;
	
	xmlNode *child = node->children;
	
	while(child != NULL){
		// TODO: check to only take nodes, not text and stuff
		
		if( !xmlNodeIsText(child) &&
		   (tag.empty() || tag == (const char *)child->name)){
			children.push_back(child);
		}
		child = child->next;
	}
	
	return children;
}

string _getContent(xmlNode *node){	
	string returnval((const char *)xmlNodeGetContent(node));
	return returnval;
}


Data _getNumber(const string &expression, 
				 const GenericDataType type) {
	switch (type){
			
		case M_FLOAT:
			return Data(boost::lexical_cast<double>(expression));
		case M_INTEGER:
			return Data(boost::lexical_cast<long>(expression));
		case M_STRING:
			return Data(expression);
		case M_BOOLEAN:
			return Data(boost::lexical_cast<bool>(expression));
	}
	
	return Data(expression);
}	


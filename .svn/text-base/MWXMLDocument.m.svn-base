//
//  MWXMLDocument.m
//  NewEditor
//
//  Created by David Cox on 11/1/07.
//  Copyright __MyCompanyName__ 2007 . All rights reserved.
//

#import "MWXMLDocument.h"

@implementation MWXMLDocument

- (id)init
{
    self = [super init];
	
	validationInProcess = NO;
	
    if (self) {
		////////////////////////////////////////////////////////////////
		
		internally_generated_count = 0;
	
        // If an error occurs here, send a [self release] message and return nil.
		
		NSError *outError;
		
		// The XML of an empty experiment
		NSString *xmlEmptyExperimentPath = [[NSBundle mainBundle]  
											pathForResource:@"MWEmptyExperiment" 
											ofType:@"xml"];
		
		// The XML containing info about MW elements
		NSString *xmlLibraryPath = [[NSBundle mainBundle]  
									pathForResource:@"MWLibrary" 
									ofType:@"xml"];
		
		// XSLT for transforming the library for display in a browser view
		NSString *xmlLibraryTransformationPath = [[NSBundle mainBundle]  
										pathForResource:@"MWLibraryTransformation" 
										ofType:@"xsl"];

		// XSLT for transforming the library into another XSLT stylesheet
		// to add display hints to the document tree
		NSString *xmlLibraryToXSLTPath = [[NSBundle mainBundle]  
										pathForResource:@"MWLibraryToXSLTransform" 
										ofType:@"xml"];
										
		NSString *schematronReportGeneratorPath = [[NSBundle mainBundle]
										pathForResource:@"schematron_report_generator"
										ofType:@"xsl"];

		NSString *schematronPath = [[NSBundle mainBundle]
										pathForResource:@"MWSchematron"
										ofType:@"xml"];

		// the XSLT for combining all of the plugins 
		NSString *xslLibraryCombiner = [[NSBundle mainBundle]  
										pathForResource:@"MWLibraryCombiner" 
										ofType:@"xsl"];
		
		
		// --------------------------------------------------------------------
		// Read in the Library XML
		// --------------------------------------------------------------------
		/////////////////////////////////////////////////////////////////////
		// Get all of the plugins' library files
		/////////////////////////////////////////////////////////////////////
		NSLog(@"Loading Library XML...");
		NSString *pluginPath = @"/Library/MonkeyWorks/Plugins";
		NSArray *plugins = [[NSFileManager defaultManager] contentsOfDirectoryAtPath:pluginPath error:&outError];
		NSEnumerator *pluginEnumerator = [plugins objectEnumerator];
		NSString *pluginName = nil;
		
		// create an xml document that contains a list of all the files that need to be parsed
		NSXMLElement *root = [[[NSXMLElement alloc] initWithName:@"contents"] autorelease];
		NSXMLNode *main_library_url_node = [NSXMLNode elementWithName:@"url" stringValue:xmlLibraryPath];
		[root addChild:main_library_url_node];
		
		while(pluginName = [pluginEnumerator nextObject]) {
			NSString *xmlPluginPath = [[NSBundle bundleWithPath:[pluginPath stringByAppendingPathComponent:pluginName]] 
									   pathForResource:@"MWLibrary" 
									   ofType:@"xml"];
			
			if(xmlPluginPath != nil) {
				NSXMLNode *library_url_node = [NSXMLNode elementWithName:@"url" stringValue:xmlPluginPath];
				[root addChild:library_url_node];
			}
		}
		
		NSXMLDocument *xmlContentsDoco = [[[NSXMLDocument alloc] initWithRootElement:root] autorelease];
		//NSLog([xmlContentsDoco XMLString]);
		
		// combine all the libraries now
		NSXMLDocument *libraries_untransformed = [xmlContentsDoco 
												  objectByApplyingXSLTAtURL:[NSURL fileURLWithPath:xslLibraryCombiner]
												  arguments:nil
												  error:&outError];
		
		// I need to write and re-read the file.  There's some bug here but I don't see where it is
		// write it to a temp file:
		CFUUIDRef uuid = CFUUIDCreate(NULL);
		NSString *uString = [(NSString *)CFUUIDCreateString(NULL, uuid) autorelease];
		CFRelease(uuid);
		NSString *temp_file = [NSTemporaryDirectory() stringByAppendingPathComponent:uString];
		[[libraries_untransformed XMLData] writeToFile:temp_file options:0 error:0];
		
		// read the temp file:
		libraries_untransformed = [[NSXMLDocument alloc] initWithContentsOfURL:[NSURL fileURLWithPath:temp_file]
																	   options: NSXMLDocumentTidyXML
																		 error:&outError];
		
		
		NSLog(@"Transforming Library XML...");
		library = [libraries_untransformed 
						objectByApplyingXSLTAtURL:[NSURL fileURLWithPath:xmlLibraryTransformationPath]
														arguments:nil
														error:&outError];
		
			   
		if ( outError != Nil ) {
			NSRunAlertPanel(@"Error", @"Error transforming library", @"OK", Nil, Nil, Nil);
			return self;
		}
		
		[[NSFileManager defaultManager] removeItemAtPath:temp_file error:nil];
		//NSLog([library XMLString]);
		
		
		// --------------------------------------------------------------------
		// Transform the Library XML into a display hint transformation
		// --------------------------------------------------------------------
		
		NSLog(@"Transforming Library XML for display hints...");
		display_hint_transformation = 
					[libraries_untransformed 
						objectByApplyingXSLTAtURL:[NSURL fileURLWithPath:xmlLibraryToXSLTPath]
														arguments:nil
														error:&outError];
			   
		if ( outError != Nil ) {
			NSRunAlertPanel(@"Error", @"Error generating library transformation", @"OK", Nil, Nil, Nil);
			return self;
		}
		NSLog([display_hint_transformation XMLString]);														
		
		// --------------------------------------------------------------------
		// Load an empty xml document
		// --------------------------------------------------------------------
																																						
		NSXMLDocument *raw_document = [[NSXMLDocument alloc] initWithContentsOfURL:
														[NSURL fileURLWithPath:xmlEmptyExperimentPath]
										  options:NSXMLDocumentTidyXML
										  error:&outError];
										  
			   
		if ( outError != Nil ) {
			NSRunAlertPanel(
				[NSString stringWithFormat:@"Error"], 
				[NSString stringWithFormat:@"Error loading default xml document:%@", [outError localizedDescription] ], 
				@"OK", Nil, Nil, Nil);
			return self;
		}
		
		
		// --------------------------------------------------------------------
		// Transform the raw xml document with the display hint transformation
		// --------------------------------------------------------------------
		
		document = [raw_document
						objectByApplyingXSLT:[display_hint_transformation XMLData]
													arguments:nil
													error:&outError];
													
			
		NSLog([document XMLString]);
		
		if ( outError != Nil ) {
			NSRunAlertPanel(@"Error", @"Error transforming xml document", @"OK", Nil, Nil, Nil);
			return self;
		}
		
		
		// --------------------------------------------------------------------
		// Create a default error document
		// --------------------------------------------------------------------
		
		NSString *defaultErrors = @"<errors><error message=\"No errors\"/></errors>";
		documentErrors = [[NSXMLDocument alloc] initWithXMLString:defaultErrors
												options:NSXMLDocumentTidyXML
												error:&outError];
		
		
		// --------------------------------------------------------------------
		// Read in the schematron
		// --------------------------------------------------------------------
		
		NSXMLDocument *schematron_untransformed = [[NSXMLDocument alloc] initWithContentsOfURL:
														[NSURL fileURLWithPath:schematronPath]
										  options:NSXMLDocumentTidyXML
										  error:&outError];
		
		
			   
		if ( outError != Nil ) {
			NSRunAlertPanel(
				[NSString stringWithFormat:@"Error"], 
				[NSString stringWithFormat:@"Error loading schematron document:%@", [outError localizedDescription] ], 
				@"OK", Nil, Nil, Nil);
			return self;
		}

		
		// --------------------------------------------------------------------
		//  Transform Schematron in XSLT
		// --------------------------------------------------------------------
		
		
		
		NSLog(@"Transforming Schematron XML into XSLT...");
		schematronTransform = 
					[schematron_untransformed 
						objectByApplyingXSLTAtURL:[NSURL fileURLWithPath:schematronReportGeneratorPath]
											arguments: Nil
											error:&outError];
		
		NSString *schematron_string = [schematronTransform XMLString];
		NSString *schematron_string_mod = [schematron_string 
						stringByReplacingOccurrencesOfString:@"axsl"
						withString:@"xsl"];
		
		schematron_string_mod = [schematron_string_mod 
						stringByReplacingOccurrencesOfString:@"TransformAlias"
						withString:@"Transform"];
                        
        //NSLog(@" ================ SCHEMATRON TRANSFORM ===================== ");
        //NSLog(schematron_string_mod);
        //NSLog(@" ------------------------------------------------------------- ");
		schematronTransform = [[NSXMLDocument alloc] initWithXMLString:schematron_string_mod
												options:NSXMLDocumentTidyXML
												error:&outError];
			   
			   
		if ( outError != Nil ) {
			NSRunAlertPanel(@"Error", @"Error generating schematron transformation", @"OK", Nil, Nil, Nil);
			return self;
		}
			
		NSLog([schematronTransform XMLString]);
				
		
		//NSLog([library XMLString]);
    }
	
	[self validateDocument];
    
    
	
    NSUserDefaultsController *defaults = [NSUserDefaultsController sharedUserDefaultsController];
    
    
    [defaults addObserver:self forKeyPath:@"values.MWAutomaticallyChecksForErrors"
                               options:NSKeyValueObservingOptionNew
                               context:nil];
    [defaults addObserver:self forKeyPath:@"values.MWAutomaticErrorCheckInterval"
                               options:NSKeyValueObservingOptionNew
                               context:nil];
    
    NSNumber *checkInterval = [[defaults values] valueForKey:@"MWAutomaticErrorCheckInterval"];
    errorCheckInterval = [checkInterval doubleValue];
                                                                                                                                                                   
    NSNumber *checkAutomatically = [[defaults values] valueForKey:@"MWAutomaticallyChecksForErrors"];
    if([checkAutomatically boolValue]){
        [self startValidationTimer];
	}
    
    
	//[NSThread detachNewThreadSelector:@selector(startValidationTimer)
	//		  toTarget:self withObject:Nil];
	
    return self;
}

- (void) observeValueForKeyPath:(NSString *)path ofObject:(id)obj change:(NSDictionary *)change
                                            context:(void *)context {
 
    if([path isEqualToString:@"values.MWAutomaticallyChecksForErrors"]){
        
        NSNumber *do_it = [obj valueForKeyPath:@"values.MWAutomaticallyChecksForErrors"];
        //NSNumber *do_it = [change objectForKey:NSKeyValueChangeNewKey]; 
        [self cancelValidationTimer];
        if([do_it boolValue]){
            [self startValidationTimer];
        } else {
            [self setDocumentErrorsForPosting:nil];
            [self validateDocument];
//            [self clearErrors:[document rootElement]];
  //          [self setValid:YES];
            
        }
    }
    
    if([path isEqualToString:@"values.MWAutomaticErrorCheckInterval"]){
        NSNumber *interval = [obj valueForKeyPath:@"values.MWAutomaticErrorCheckInterval"];
        errorCheckInterval = [interval doubleValue];
        
        if([[obj valueForKey:@"values.MWAutomaticallyChecksForErrors"] boolValue]){
            [self cancelValidationTimer];
            [self startValidationTimer];
        }
    }
            
}


- (void) dealloc {

	[validationTimer invalidate];
	[super dealloc];
}

- (NSString *)windowNibName
{
    // Override returning the nib file name of the document
    // If you need to use a subclass of NSWindowController or if your document supports multiple NSWindowControllers, you should remove this method and override -makeWindowControllers instead.
    return @"MyDocument";
}

- (void)windowControllerDidLoadNib:(NSWindowController *) aController
{
    [super windowControllerDidLoadNib:aController];
    // Add any code here that needs to be executed once the windowController has loaded the document's window.
}


- (NSXMLDocument *)document {
	return document;
}

- (void)setDocument:(NSXMLDocument *)doc{
	document = doc;
}

- (NSXMLDocument *)library {
	return library;
}

- (NSArray *) allLibraryElements{

    NSError *error;
    NSArray *nodes = [library nodesForXPath:@"//MWElementGroup[@name='All']/*" error:&error];
    return nodes;
}


- (void)setLibrary:(NSXMLDocument *)lib{
    [self willChangeValueForKey:@"allLibraryElements"];
	library = lib;
    [self didChangeValueForKey:@"allLibraryElements"];
}


- (NSXMLDocument *) displayHintTransform{
	return display_hint_transformation;
}

- (NSString *)displayHintTransformString{
	return [display_hint_transformation XMLString];
}


- (NSString *)newInternalID {
	return [NSString stringWithFormat:@"_id%d", internally_generated_count++];
}


- (NSData *)dataOfType:(NSString *)typeName error:(NSError **)outError
{
    // Insert code here to write your document to data of the specified type. If the given outError != NULL, ensure that you set *outError when returning nil.

    // You can also choose to override -fileWrapperOfType:error:, -writeToURL:ofType:error:, or -writeToURL:ofType:forSaveOperation:originalContentsURL:error: instead.

    // For applications targeted for Panther or earlier systems, you should use the deprecated API -dataRepresentationOfType:. In this case you can also choose to override -fileWrapperRepresentationOfType: or -writeToFile:ofType: instead.
	NSLog(@"Type name = %@", typeName);

	// XSLT for transforming the library into another XSLT stylesheet
	// to add display hints to the document tree
	NSString *xmlStripTransformationPath = [[NSBundle mainBundle]  
										pathForResource:@"MWStripHintsTransformation" 
										ofType:@"xml"];

	NSXMLDocument *stripped_doc = [document objectByApplyingXSLTAtURL:[NSURL fileURLWithPath:xmlStripTransformationPath]
													arguments:nil
													error:outError];
	if(stripped_doc == Nil){
		NSRunAlertPanel(@"Error", @"Error writing file", @"OK", Nil, Nil, Nil);
	}
	
	*outError = Nil;
	return [stripped_doc XMLDataWithOptions:NSXMLNodePrettyPrint];
	
    //if ( outError != NULL ) {
//		*outError = [NSError errorWithDomain:NSOSStatusErrorDomain code:unimpErr userInfo:NULL];
//	}
//	return nil;
}


- (BOOL)readFromData:(NSData *)data ofType:(NSString *)typeName error:(NSError **)outError
{
    
	NSError *error;
	
	if(data == Nil){
		document = Nil;
		return NO;
	} else {
		
		// read in the doc
		NSXMLDocument *raw_document = [[NSXMLDocument alloc] initWithData:data options:NSXMLDocumentTidyXML error:outError];
		
		// apply display hints (e.g. "_hint")
		document = [raw_document
						objectByApplyingXSLT:[display_hint_transformation XMLData]
													arguments:Nil
													error:&error];
	}
	
    if ( document == Nil ) {
		
		NSRunAlertPanel(@"Error creating XML document", @"blah", @"OK", Nil, Nil, Nil);
		//NSLog(@"Oh shit: %@", [error localizedDescription]);
		
	}
	
	
    return YES;
}



#pragma mark -
#pragma mark Document Validation Methods

- (void)startValidationTimer {
	validationTimer = [NSTimer scheduledTimerWithTimeInterval: errorCheckInterval
				 target: self
                 selector: @selector(validateDocument)
                 userInfo: nil
                 repeats: YES];
		
}

- (void)cancelValidationTimer {
    [validationTimer invalidate];
}

- (BOOL)valid {
	return isValid;
}

- (void)setValid:(BOOL)_isValid{
	isValid = _isValid;
}

// Clear "_error" hints in the document tree
- (void) clearErrors :(NSXMLElement *)element {

	if([element attributeForName:@"_error"]){
		[element setValue:@"" forKey:@"_error"];
		[element removeAttributeForName:@"_error"];
	}
	
	NSArray *children = [element children];
	NSEnumerator *child_enumerator = [children objectEnumerator];
	NSXMLElement *child_element;
	while(child_element = [child_enumerator nextObject]){
		[self clearErrors:child_element];
	}
}


// Based on the documentErrors xml document, mark up the main document XML
// tree with "_error" hint attributes
- (void) markErrors {
    if(documentErrors == nil){
        return;
    }

	NSArray *error_nodes = [[documentErrors rootElement] children];
	NSEnumerator *error_enumerator = [error_nodes objectEnumerator];
	
	NSError *outError;
	
	NSXMLElement *error_node;
	while(error_node = [error_enumerator nextObject]){
		NSString *node_id = [error_node valueForKey:@"id"];
		//NSLog(@"Marking node id %@", node_id);
		NSString *error_message = [error_node valueForKey:@"message"];
		NSString *xpath = [NSString stringWithFormat:@"//*[@_id = '%@']", node_id]; 
		//NSString *xpath = [NSString stringWithFormat:@"//variable"];
		
		NSArray *matching_nodes = [document nodesForXPath:xpath error:&outError];
		
		
		/*if(outError != Nil){
			NSLog(@"bad xpath");
		}
		NSLog(@"%d matching nodes (%@)", [matching_nodes count], xpath);*/
		
		NSEnumerator *matching_node_enumerator = [matching_nodes objectEnumerator];
		NSXMLElement *matching_node;
		while(matching_node = [matching_node_enumerator nextObject]){
			NSXMLNode *new_att = [NSXMLNode attributeWithName:@"_error"
											stringValue:error_message];
			[matching_node addAttribute:new_att];
			[matching_node setValue:error_message forKey:@"_error"];
			//NSLog(@"ammended: %@", [matching_node XMLString]);
		}
	}
}


// Run a schematron (and possibly other validation procedures) on the main
// XML document, and save the results in the documentErrors XML document
- (void) validateDocument {

//	NSLog(@"validating");
	
	NSXMLDocument *new_errors;			
	@synchronized(newErrorsForPosting){
        if(newErrorsForPosting != nil){
            new_errors = [newErrorsForPosting copyWithZone:Nil];
        } else {
            new_errors = nil;
        }
	}
	
	
	[self setDocumentErrors:new_errors];
		
	if(new_errors != nil && [[[self documentErrors] rootElement] childCount] != 0){
		[self setValid:NO];
	} else {
		[self setValid:YES];
	}
	
	// Update "_error" display hints in the XML document
	[self clearErrors:[document rootElement]];
	[self markErrors];

	
	@synchronized(validationInProcess){
		if([validationInProcess boolValue]){
			return;
		}
	}
	
	@synchronized(documentCopyForValidation){
		documentCopyForValidation = [document copyWithZone:Nil];
	}
	
	[NSThread detachNewThreadSelector:@selector(doValidationWork)
			  toTarget:self withObject:Nil];
	
	return;
}

- (void) doValidationWork {

	@synchronized(validationInProcess){
		validationInProcess = [NSNumber numberWithBool:YES];
	}

	//NSLog(@"validating");
	NSError *outError;
	
	// Run the XSLT-compiled schematron on the main document
	NSXMLDocument *errors = [documentCopyForValidation  objectByApplyingXSLT:[schematronTransform XMLData]
													arguments:nil
													error:&outError];
	
	
	[self setDocumentErrorsForPosting:errors];
	
	//NSLog(@"Finished real validation work");

                    		
	@synchronized(validationInProcess){
		validationInProcess = NO;
	}
    
    
    
}

- (void)postErrorNotification {
    [[NSNotificationCenter defaultCenter] postNotificationName:MW_DOCUMENT_ERRORS_NOTE object:self];
}

- (NSXMLDocument *)documentErrors {
	NSXMLDocument *doc;
	doc = documentErrors;
	return doc;
}

- (void)setDocumentErrors: (NSXMLDocument *)_documentErrors {

    NSXMLDocument *originalErrors = documentErrors;
	documentErrors = _documentErrors;
    if([[documentErrors rootElement] childCount] != [[originalErrors rootElement] childCount]){
        [self performSelectorOnMainThread:@selector(postErrorNotification) withObject:nil waitUntilDone:YES];
    }
}

- (void)setDocumentErrorsForPosting: (NSXMLDocument *)_documentErrors {
	
	@synchronized(newErrorsForPosting){
		newErrorsForPosting = [_documentErrors copyWithZone:Nil];
        
	}
	
}


@end

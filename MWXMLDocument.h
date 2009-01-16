//
//  MWXMLDocument.h
//  NewEditor
//
//  Created by David Cox on 11/1/07.
//  Copyright __MyCompanyName__ 2007 . All rights reserved.
//


#import <Cocoa/Cocoa.h>

#define MW_DOCUMENT_ERRORS_NOTE  @"MW_DOCUMENT_ERRORS_NOTE"

@interface MWXMLDocument : NSDocument
{
	// An XML representation of a user experiment
	NSXMLDocument *document;
	
	// An XML representation of element definitions, validation rules,
	// code templates, descriptions, etc.  This information populates the
	// "Library" browser view
	NSXMLDocument *library;
	
	// An XML document chronicling errors, if there are any
	// This will be populated by schematron evaluation, among other routes
	NSXMLDocument *documentErrors;

	// An XSLT document for adding display hints to a MW XML document
	NSXMLDocument *display_hint_transformation;
	
	// An XSLT document that applies schematron validation
	NSXMLDocument *schematronTransform;
	
	NSTimer *validationTimer;
	
	// Validation of big documents will be expensive, so it needs to happen
	// in another thread.  Since AppKit + bindings are not thread safe, we'll
	// need to be continually making copies
	NSXMLDocument *documentCopyForValidation;
	NSXMLDocument *newErrorsForPosting;
	NSNumber *newErrorsAvailableForPosting;
	NSNumber *validationInProcess;
	
    double errorCheckInterval;
    
	BOOL isValid;
	
	int internally_generated_count;
	
	
	
}

- (NSXMLDocument *)library;
- (NSXMLDocument *) document;
- (NSArray *) allLibraryElements;
- (NSXMLDocument *) documentErrors;
- (NSXMLDocument *) displayHintTransform;
- (NSString *)displayHintTransformString;

- (void) validateDocument;

- (NSString *)newInternalID;

- (void)setDocumentErrors: (NSXMLDocument *)_documentErrors;
- (void)setDocumentErrorsForPosting: (NSXMLDocument *)_documentErrors;


- (void)startValidationTimer;
- (void)cancelValidationTimer;

@end

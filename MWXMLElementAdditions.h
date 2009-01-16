//
//  MWXMLNodeAdditions.h
//  NewEditor
//
//  Created by David Cox on 11/1/07.
//  Copyright 2007 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface NSXMLElement (MWXMLElementAdditions)


// Additions to make the NSXMLElement more Key-Value-Coding Compliant, for use with
// Cocoa bindings.  This override first checks if the node has an attribute
// with the specified key; failing this, it falls back to the superclass
// valueForKey methods
- (id)valueForKey:(NSString *)key;
- (void)setValue:(id)val forKey:(NSString *)key;

- (NSString *)displayName;

- (BOOL)leaf;
- (void)setLeaf:(BOOL)_leaf;

- (NSString *)prettyXMLString;

- (BOOL)hasAttribute:(NSString *)name;

- (NSArray *)ancestors;
- (NSIndexSet *)indexSetInAncestorArray;
- (void)setIndexSetInAncestorArray:(id)set;


@end

//
//  MWXMLNodeAdditions.m
//  NewEditor
//
//  Created by David Cox on 11/1/07.
//  Copyright 2007 __MyCompanyName__. All rights reserved.
//

#import "MWXMLElementAdditions.h"


@implementation NSXMLElement (MWXMLElementAdditions)


- (id)object {
	return self;
}

- (void)setObject:(id)object {
	// actually just swap out the tag
	//if([object isKindOfClass:@"NSString"]){
	NSLog(@"setting");
	[self setValue:object forKey:@"tag"];
	//}
	return;
}

- (BOOL)leaf{
	return [self childCount] == 0;
}

- (void)setLeaf:(BOOL)_leaf{
	return;
}

- (NSString *)displayName {
	NSXMLNode *att = [self attributeForName:@"tag"];
	if(att == Nil){
		return [self name];
	}

	return [att stringValue];
}

- (void)setDisplayName {
	return;
}

- (id)valueForKey:(NSString *)key{
	NSXMLNode *att = [self attributeForName:key];
	if(att == Nil){
		return [super valueForKey:key];
	}

	NSString *returnval;
	
	returnval = [att stringValue];
	
	return returnval;
}

- (id)valueForUndefinedKey:(NSString *)key{
	return Nil;
	//return @"Undefined";
}


- (void)setValue:(id)val forKey:(NSString *)key{

	NSXMLNode *att = [self attributeForName:key];
	if(att == Nil){
	
		if([self respondsToSelector:NSSelectorFromString(key)]){
			[super setValue:val forKey:key];
			return;
		}
		[self setValue:val forUndefinedKey:key];
		return;
	}

	[self willChangeValueForKey:key];
	[att willChangeValueForKey:@"stringValue"];
	[self willChangeValueForKey:@"prettyXMLString"];
	[self willChangeValueForKey:@"object"];
	
	
	// TODO be sure to convert val to string
	[att setStringValue:val resolvingEntities:NO];
	
	[self didChangeValueForKey:key];
	[att didChangeValueForKey:@"stringValue"];
	[self didChangeValueForKey:@"prettyXMLString"];
	
	[self didChangeValueForKey:@"object"];


	return;
}

- (void)setValue:(id)val forUndefinedKey:(NSString *)key{

	NSLog(@"creating attribute");
	
	NSXMLNode *new_att = [NSXMLNode attributeWithName:key stringValue:val];
	
	[self willChangeValueForKey:key];
	[new_att willChangeValueForKey:@"displayName"];
	[self willChangeValueForKey:@"prettyXMLString"];
	
	// ddc removed, didn't seem important
	[self willChangeValueForKey:@"object"];
	
    
	[self addAttribute:new_att];
	
	[self didChangeValueForKey:key];
	[new_att didChangeValueForKey:@"displayName"];
	[self didChangeValueForKey:@"prettyXMLString"];
	
	// ddc removed, didn't seem important
	[self didChangeValueForKey:@"object"];
	
	
}

	
- (NSString *)prettyXMLString{
	return [self XMLStringWithOptions:NSXMLNodePrettyPrint|NSXMLNodePreserveWhitespace];
}


/*- (NSString *)stringValue {
	NSXMLNode *att = [self attributeForName:@"tag"];
	if(att == Nil){
		return [self prettyXMLString];
	}

	return [att stringValue];
}*/

- (NSArray *)observedAttributes {

	NSArray *returnval = [self attributes];
	/*int i;
	for(i = 0; i < [returnval count]; i++){
		[[returnval objectAtIndex:i] addObserver:self forKeyPath:@"stringValue"
	                 options:(NSKeyValueObservingOptionNew |
                            NSKeyValueObservingOptionOld) 
                    context:NULL];
	}
	*/
	return returnval;
}

- (BOOL) hasAttribute: (NSString *)attname {
	NSXMLNode *att = [self attributeForName:attname];
	return (att != Nil);
}

- (NSArray *)childrenExt{
	
	NSArray *chillens = [self children];
	if(chillens == Nil){
		return [NSMutableArray array];
	}
	
	return chillens;
}

- (void)setChildrenExt: (NSArray *)newchildren {

	[self setChildren:newchildren]; 
}

- (NSArray *)ancestors {

	NSMutableArray *ancestor_array = [[NSMutableArray alloc] init];
	
	
	[ancestor_array removeAllObjects];
	
	[ancestor_array addObject:self];
	
	NSXMLNode *current = self;
	while([current parent] != Nil && 
		  [current parent] != [[current rootDocument] rootElement] && 
		  ![[current parent] isKindOfClass:[NSXMLDocument class]]){
		[ancestor_array insertObject:(NSXMLElement *)[current parent] atIndex:0];
		current = [current parent];
	}
	
	return ancestor_array;
}

- (NSIndexSet *)indexSetInAncestorArray {
	NSArray *ancestors = [self ancestors];
	return [NSIndexSet indexSetWithIndex:[ancestors count]-1];
}

- (void)setIndexSetInAncestorArray:(id)set {
	// do nada
}


@end

//
//  NRTPlotObject.h
//  Narrative
//
//  This is a super class for all non-view plotting classes.
//  It's purpose is to store attributes.
//
//  Created by Drew McCormack on Tue Sep 03 2002.
//
//  Narrative -- a plotting framework for Cocoa and GNUStep. 
//  Copyright (C) 2003 Drew McCormack
// 
//  This library is free software; you can redistribute it 
//  and/or modify it under the terms of the GNU Lesser General 
//  Public License as published by the Free Software 
//  Foundation; either version 2.1 of the License, or 
//  (at your option) any later version. 
//
//  This library is distributed in the hope that it will be 
//  useful, but WITHOUT ANY WARRANTY; without even the implied 
//  warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
//  See the GNU Lesser General Public License for more details. 
// 
//  You should have received a copy of the GNU Lesser General Public 
//  License along with this library; if not, write to the Free 
//  Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
//  MA  02111-1307  USA 
//

#import <Cocoa/Cocoa.h>


// Attribute keys
extern NSString *NRTLineWidthAttrib;
extern NSString *NRTLineColorAttrib;
extern NSString *NRTFillColorAttrib;


@interface NRTPlotObject : NSObject {
    @private
    NSMutableDictionary *_attributesDictionary;
}

+(void)initialize;

-(id)init;
-(void)dealloc;

-(NSMutableDictionary *)attributesDictionary;
-(void)setAttributesDictionary:(NSMutableDictionary *)dict;

-(id)attributeForKey:(NSObject <NSCopying> *)key;
-(void)setAttribute:(id)obj forKey:(NSObject <NSCopying> *)key;

@end

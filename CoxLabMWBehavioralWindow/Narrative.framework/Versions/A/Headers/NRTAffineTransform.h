//
//  NRTAffineTransform.h
//  Narrative
//
//  Equivalent to the NSAffineTransform. It transforms from one
//  2D coordinate system to another, allowing for rotations, translations,
//  and scaling.
//
//  Created by Drew McCormack on Wed Aug 14 2002.
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
#import "NRTIrreducibleTransform.h"


@interface NRTAffineTransform : NRTIrreducibleTransform {
    NSAffineTransform 	*_transform;
}

-(id)initWithBaseIdentifiers:(NSArray *)baseCoords
    andTransformedIdentifiers:(NSArray *)transformedCoords;
-(void)dealloc;

-(NSAffineTransform *)transform;
-(void)setTransform:(NSAffineTransform *)trans;

-(unsigned)numberOfBaseCoordinates;
-(unsigned)numberOfTransformedCoordinates;

-(NSDictionary *)transformedCoordinatesForBaseCoordinates:(NSDictionary *)coords;

@end

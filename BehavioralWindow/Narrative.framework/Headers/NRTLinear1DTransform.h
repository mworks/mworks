//
//  NRTLinear1DTransform.h
//  Narrative
//
//  A 1D linear transform.
//
//  Created by Drew McCormack on Sun Sep 01 2002.
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

#import <Foundation/Foundation.h>
#import "NRTIrreducibleTransform.h"


@interface NRTLinear1DTransform : NRTIrreducibleTransform {
    float _translation, _scalingFactor;
}

-(id)initWithBaseIdentifiers:(NSArray *)baseCoords
    andTransformedIdentifiers:(NSArray *)transformedCoords;

-(void)setTranslation:(float)translation;
-(float)translation;
-(void)setScalingFactor:(float)scale;
-(float)scalingFactor;

-(NSDictionary *)transformedCoordinatesForBaseCoordinates:(NSDictionary *)coords;

-(unsigned)numberOfBaseCoordinates;
-(unsigned)numberOfTransformedCoordinates;

@end

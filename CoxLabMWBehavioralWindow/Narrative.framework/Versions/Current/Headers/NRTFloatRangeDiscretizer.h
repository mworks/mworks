//
//  NRTFloatRangeDiscretizer.h
//  Narrative
//
//  In this default implementation, a user can choose whether or not to have
//  discrete intervals rounded so that the interval only has a single significant
//  non-zero figure. For example, 1, 10, 500, 400000 are numbers of this type. If
//  this option is used, the discretizer may change the number of points in the
//  range to accommodate for this.
//
//  Created by Drew McCormack on Sat Oct 05 2002.
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
#import "NRTDiscretizer.h"
#import "NRTFloatRange.h"


@interface NRTFloatRangeDiscretizer : NRTDiscretizer {
    @private
    NRTFloatRange 	_floatRange;
    BOOL	_useOnlyRoundNumbers;		// Uses only one significant (non-zero) figure (eg 1, 10, 500)
    unsigned	_desiredNumberOfDiscretePoints;	// Different discretizers can interpret this differently.
}

// Designated
-(id)initWithFloatRange:(NRTFloatRange)range desiredNumberOfPoints:(unsigned)points 
    useOnlyRoundNumbers:(BOOL)round;
-(id)initWithFloatRange:(NRTFloatRange)range desiredNumberOfPoints:(unsigned)points; // No rounding
-(id)initWithFloatRange:(NRTFloatRange)range;	// No rounding and 10 points

-(void)setFloatRange:(NRTFloatRange)range;
-(NRTFloatRange)floatRange;

-(void)setDesiredNumberOfDiscretePoints:(unsigned)num;
-(unsigned)desiredNumberOfDiscretePoints;

-(void)setUseOnlyRoundNumbers:(BOOL)roundNumbers;
-(BOOL)useOnlyRoundNumbers;

-(NSArray *)discretePoints;

@end

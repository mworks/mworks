//
//  NRTStraightAxis.h
//  Narrative
//
//  Created by Drew McCormack on Tue Jul 16 2002.
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
#import "NRTAxis.h"

@class NRTAxisTick;

@interface NRTStraightAxis : NRTAxis {
    NSPoint _startViewPoint, _endViewPoint;  
}

// Designated Constructor
-(id)initWithCoordinateIdentifier:(id)coordIdentifier
    axisIdentifier:(id)axisIdentifier
    startViewPoint:(NSPoint)startPoint
    endViewPoint:(NSPoint)endPoint;

-(NSPoint)startViewPoint;
-(void)setStartViewPoint:(NSPoint)point;
-(NSPoint)endViewPoint;
-(void)setEndViewPoint:(NSPoint)point;

-(NSPoint)principalPoint;

-(void)drawInPlotView:(NRTPlotView *)view;

-(NSPoint)plotViewPointForAxisCoordinate:(float)coord;

@end

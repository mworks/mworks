//
//  NRTAxis.h
//  Narrative
//
//  An NRTAxis is one dimensional, and has a coordinate which ranges from 0 to 1, called
//  the "axis coordinate". It simply has a linear dependence on the position along
//  the axis length.
//  For convenience, tick placements can be given in plotting coordinates, which are
//  then transformed to the axis coordinate for drawing by the NRTAxis via a NRTTransform
//  object.
//
//  Created by Drew McCormack on Sat May 25 2002.
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
#import "NRTPlotViewComponent.h"
#import "NRTPlotObject.h"

@class NRTPlotView;
@class NRTTransform;
@class NRTAxisTitle;

@interface NRTAxis : NRTPlotObject <NRTPlotViewComponent>
{
    NSArray 			*_tickPositions;  // Given in plot coordinate, not axis coord
    NSArray			*_patternOfTicks; // Can be all ticks, or just the repeating pattern
    NSArray 			*_tickLabels;
    NSObject <NSCopying> 	*_identifier;           // An identifier for the axis
    NSObject <NSCopying> 	*_coordinateIdentifier; // The identifier of the coord. to which the axis applies
    NRTTransform			*_plotToAxisCoordinateTransform; // Transforms from plot coord to axis coord
    NRTAxisTitle			*_title;
}

-(id)initWithCoordinateIdentifier:(id)coordIdentifier
    axisIdentifier:(id)axisIdentifier;
-(id)init;
-(void)dealloc;

-(NSObject <NSCopying> *)identifier;
-(void)setIdentifier:(NSObject <NSCopying> *)identifier;

-(NSObject <NSCopying> *)coordinateIdentifier;
-(void)setCoordinateIdentifier:(NSObject <NSCopying> *)coordIdentifier;

-(void)drawInPlotView:(NRTPlotView *)view;

// Principal point is used as an origin for aligning objects. Abstract.
-(NSPoint)principalPoint;

// Ticks
-(NSArray *)tickPositions;			// Position is given in plotting coordinates
-(void)setTickPositions:(NSArray *)positions;

// This is one or more ticks, which represent the repeating pattern used to draw all ticks on the axis.
-(NSArray *)patternOfTicks;			
-(void)setPatternOfTicks:(NSArray *)tickPattern;

// Labels
-(NSArray *)tickLabels;
-(void)setTickLabels:(NSArray *)labels;

// NRTTitle of axis
-(NRTAxisTitle *)title;
-(void)setTitle:(NRTAxisTitle *)title;

// NRTTransform
-(NRTTransform *)plotToAxisCoordinateTransform;
-(void)setPlotToAxisCoordinateTransform:(NRTTransform *)transform;

// Abstract methods used for determining drawing coords.
-(NSPoint)plotViewPointForAxisCoordinate:(float)coord;

@end

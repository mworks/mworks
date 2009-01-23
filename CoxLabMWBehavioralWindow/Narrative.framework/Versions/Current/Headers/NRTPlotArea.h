//
//  NRTPlotArea.h
//  Narrative
//
//  This is an NSView where the plots are drawn, but also 
//  converts plotting coordinates to NSView coordinates.
//  This particular class represents a rectangular area filling
//  the entire NSView area. By subclassing you can make more exotic
//  areas, like circles, by clipping drawing to a given area, and
//  changing the coordinates-to-view transform.
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
#import "NRTPlotView.h"

@class NRTTransform;
@protocol NRTPlotAreaComponent;

@interface NRTPlotArea : NRTPlotView 
{
    @private
    NRTTransform 	*_plotCoordinateToViewTransform;
    NSMutableArray	*_plotAreaComponents;
}

-(id)initWithFrame:(NSRect)frame;
-(void)dealloc;

-(void)setPlotCoordinateToViewTransform:(NRTTransform *)trans;
-(NRTTransform *)plotCoordinateToViewTransform;

// Sets the components coords-to-view transform, and as well as
// adding the component to the NRTPlotView's list.
-(void)addPlotAreaComponent:(NSObject <NRTPlotAreaComponent> *)component;
-(void)removeAllComponents;

-(NSMutableArray *)plotAreaComponents;
-(void)setPlotAreaComponents:(NSMutableArray *)components;

// Transforms to NSView coordinates from plotting coordinates.
-(NSPoint)plotViewPointForPlotCoordinates:(NSDictionary *)coords;

@end

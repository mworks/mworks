//
//  NRTDataClusterGraphic.h
//  Narrative
//
//  A data cluster is a logically grouped set of data in a plot. For example,
//  a point with error bars is a data cluster. Usually a data cluster requires
//  several coordinates to define it. In our error bar example, you need the
//  actual data point, plus upper and lower error bar data.
//
//  It's important not to confuse cluster coordinates, which define a whole
//  data cluster, like in the error bar example above, with plot coordinates,
//  which define coordinates in the NRTPlotArea. Typically, a data cluster will
//  require several plot coordinates to fully define it.
//
//  Created by Drew McCormack on Sat Jul 13 2002.
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
#import "NRTPlotAreaComponent.h"
#import "NRTPlotObject.h"

@class NRTTransform;

@interface NRTDataClusterGraphic : NRTPlotObject {
    @private
    NSDictionary		*_clusterCoordinates;
    NRTTransform 		*_coordinatesToViewTransform;
}

-(void)dealloc;

-(NSDictionary *)principalCoordinates; // Most important plot point in cluster. Abstract

-(void)setCoordinatesToViewTransform:(NRTTransform *)transform;
-(NRTTransform *)coordinatesToViewTransform;

-(void)drawInPlotView:(NRTPlotView *)plotView; // Abstract

-(void)setClusterCoordinates:(NSDictionary *)coords;
-(NSDictionary *)clusterCoordinates;

@end

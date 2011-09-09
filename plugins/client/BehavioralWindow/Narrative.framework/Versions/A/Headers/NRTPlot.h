//
//  NRTPlot.h
//  Narrative
//
//  Abstract class representing a single plot in a plot area.
//
//  Created by Drew McCormack on Thu Jan 01 1970.
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
//  MA  02111-1307  USA //

#import <Foundation/Foundation.h>
#import "NRTPlotAreaComponent.h"
#import "NRTPlotObject.h"

@class NRTDataClusterGraphic;
@class NRTTransform;

@interface NRTPlot : NRTPlotObject <NRTPlotAreaComponent>
{
    id 					_dataSource;
    NSObject <NSCopying>		*_identifier;
    NRTDataClusterGraphic		*_clusterGraphic;
    NRTTransform 			*_coordinatesToViewTransform;
}

-(id)initWithIdentifier:(NSObject <NSCopying> *)identifier andDataSource:(id)dataSource;
-(id)init;
-(void)dealloc;

// Abstract.
-(void)drawInPlotView:(NRTPlotView *)view;

-(void)setIdentifier:(NSObject <NSCopying> *)ident;
-(NSObject <NSCopying> *)identifier;

-(void)setDataSource:(id)ds;
-(id)dataSource;
-(BOOL)dataSourceIsSetupToAllowDrawing;

-(void)setClusterGraphic:(NRTDataClusterGraphic *)graphic;
-(NRTDataClusterGraphic *)clusterGraphic;

-(void)setCoordinatesToViewTransform:(NRTTransform *)transform;
-(NRTTransform *)coordinatesToViewTransform;

@end

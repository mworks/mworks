//
//  NRTChart.h
//  Narrative
//
//  Charts represent high-level configurations of plot elements. For
//  example, you could have a 2D chart which includes axes, ticks, labels,
//  titles, and plots. This is basically the facade design pattern. It
//  means you don't have to put all the plot elements together manually, but
//  instead have a pre-packaged chart.
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
//  MA  02111-1307  USA //

#import <Cocoa/Cocoa.h>
#import "NRTPlotView.h"

@class NRTPlotArea;
@class NRTPlot;
@class NRTAxis;
@class NRTPlotViewAnnotation;
@class NRTChart;


// Delegate methods
@interface NSObject (NRTChartDelegateMethods)

-(BOOL)chartShouldCreatePlotArea:(NRTChart *)chart;
-(void)chart:(NRTChart *)chart didCreatePlotArea:(NRTPlotArea *)plotArea;

-(BOOL)chart:(NRTChart *)chart shouldAddPlot:(NRTPlot *)plot toPlotArea:(NRTPlotArea *)plotArea;
-(void)chart:(NRTChart *)chart didAddPlot:(NRTPlot *)plot toPlotArea:(NRTPlotArea *)plotArea;

-(BOOL)chartShouldCreateTitle:(NRTChart *)chart;
-(void)chart:(NRTChart *)chart didCreateTitle:(NRTPlotViewAnnotation *)title;

-(BOOL)chartShouldCreatePlotViewAxes:(NRTChart *)chart;
-(void)chart:(NRTChart *)chart didCreatePlotViewAxes:(NSArray *)plotAreaAxes;

-(BOOL)chartShouldCreatePlotAreaAxes:(NRTChart *)chart;
-(void)chart:(NRTChart *)chart didCreatePlotAreaAxes:(NSArray *)plotAreaAxes;

@end


@interface NRTChart : NRTPlotView {
    NRTPlotArea 	*_plotArea;
    NSMutableDictionary *_attributesDictionary;
    NSMutableArray	*_plots;
    IBOutlet id		_delegate;
}

-(id)initWithFrame:(NSRect)frame;
-(void)dealloc;

-(void)setDelegate:(id)delegate;
-(id)delegate;

// Attribute methods
+(NSDictionary *)defaultAttributesDictionary;

-(NSMutableDictionary *)attributesDictionary;
-(void)setAttributesDictionary:(NSMutableDictionary *)dict;

-(void)setAttributesFromDictionary:(NSDictionary *)dict;
-(id)attributeForKey:(NSObject <NSCopying> *)key;
-(void)setAttribute:(id)obj forKey:(NSObject <NSCopying> *)key;

// Accessors
-(void)setPlotArea:(NRTPlotArea *)plotArea;
-(NRTPlotArea *)plotArea;

// Adding plots
-(void)addPlot:(NRTPlot *)plot;
-(void)removeAllPlots;

-(NSMutableArray *)plots;
-(void)setPlots:(NSMutableArray *)plots;

// Factory methods for creating/setting aspects of the chart
-(void)setChartsOwnAttributes;
-(NRTPlotArea *)createPlotArea;
-(NRTPlotViewAnnotation *)createTitle;

// Abstract factories
-(NSArray *)createPlotViewAxes;
-(NSArray *)createPlotAreaAxes;

@end

//
//  NRTPlotView.h
//  Narrative
//
//  This class basically keeps track of things which need to be drawn on 
//  it, and calls those objects to draw when drawRect: is called.
//  TODO: Add layering. Give each component a layer number to indictate
//        where it is in the layering scheme.
//
//  Created by Drew McCormack on Sat Aug 10 2002.
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

@protocol NRTPlotViewComponent;

@interface NRTPlotView : NSView 
{
    @private
    NSMutableArray 	*_componentArray;
    NSColor 		*_backgroundColor;
    NSImage		*_cachedImage;
    BOOL		_useImageCache;
    BOOL		_needsLayout;
}

-(id)initWithFrame:(NSRect)frame;
-(void)dealloc;

// Accessors
-(void)setComponentArray:(NSMutableArray *)array;
-(NSMutableArray *)componentArray;

-(void)setBackgroundColor:(NSColor *)color;
-(NSColor *)backgroundColor;

// Adding and removing components
-(void)addComponent:(NSObject <NRTPlotViewComponent> *)component;
-(void)removeAllComponents;

// TODO:
//-(NSObject <NRTPlotViewComponent> *)componentWithIdentifier:(id)componentId;
//-(void)removeComponentWithIdentifier:(id)componentId;

// Drawing
-(void)drawRect:(NSRect)rect;
-(BOOL)isOpaque;

// layout methods
-(void)setNeedsLayout:(BOOL)needsLayout;
-(BOOL)needsLayout;

-(void)layout;

// Optimizing drawing through caching
-(void)setUseImageCache:(BOOL)useCache;
-(BOOL)useImageCache;

@end

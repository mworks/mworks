//
//  NRTAnnotation.h
//  Narrative
//
//  This represents a piece of text with a position and orientation. The position 
//  and orientation are defined using an NSAffineTransform.
//
//  Created by Drew A. McCormack on Mon Sep 02 2002.
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
#import "NRTPlotObject.h"

@class NRTPlotView;

@interface NRTAnnotation : NRTPlotObject
{
    @private
    NSAttributedString		*_attributedString;
    NSAffineTransform		*_affineTransform; // Gets applied before drawing. Use for rotations etc
}

+(id)annotationWithAttributedString:(NSAttributedString *)string andAffineTransform:(NSAffineTransform *)trans;
+(id)annotationWithAttributedString:(NSAttributedString *)string;
-(id)initWithAttributedString:(NSAttributedString *)string andAffineTransform:(NSAffineTransform *)trans;
-(id)initWithAttributedString:(NSAttributedString *)string;
-(void)dealloc;

-(void)drawInPlotView:(NRTPlotView *)view atPoint:(NSPoint)point;

-(NSAffineTransform *)affineTransform;
-(void)setAffineTransform:(NSAffineTransform *)trans;

-(NSAttributedString *)attributedString;
-(void)setAttributedString:(NSAttributedString *)string;

@end

//
//  NRTSymbolClusterGraphic.h
//  Narrative
//
//  Represents a single point, possibly marked with a symbol.
//  The symbol is a bezier path, and can be filled or not.
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

#import <Cocoa/Cocoa.h>
#import "NRTDataClusterGraphic.h"

@interface NRTSymbolClusterGraphic : NRTDataClusterGraphic 
{
    @private
    NSBezierPath	*_bezierPath;
    NSSize 		_size;  // In NSView coordinates
}

-(id)initWithBezierPath:(NSBezierPath *)path;
-(void)dealloc;

-(NSSize)size;
-(void)setSize:(NSSize)size;

-(NSBezierPath *)bezierPath;
-(void)setBezierPath:(NSBezierPath *)path;

-(NSDictionary *)principalCoordinates; 

-(void)drawInPlotView:(NRTPlotView *)plotView;

@end

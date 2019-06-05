//
//  MWImageViewerImageView.m
//  ImageViewer
//
//  Created by Christopher Stawarz on 6/6/19.
//  Copyright © 2019 The MWorks Project. All rights reserved.
//

#import "MWImageViewerImageView.h"


@implementation MWImageViewerImageView {
    NSPoint selectionStartPoint;
}


- (void)setRegionOfInterest:(NSRect)regionOfInterest {
    if (!NSEqualRects(_regionOfInterest, regionOfInterest)) {
        _regionOfInterest = regionOfInterest;
        self.needsDisplay = YES;
    }
}


- (void)setSelectedRegion:(NSRect)selectedRegion {
    if (!NSEqualRects(_selectedRegion, selectedRegion)) {
        _selectedRegion = selectedRegion;
        self.needsDisplay = YES;
    }
}


static void drawScaledRect(NSRect rect, NSRect bounds, NSColor *color) {
    if (!NSIsEmptyRect(rect)) {
        [color set];
        NSFrameRect(NSMakeRect(NSMinX(rect) * NSWidth(bounds),
                               NSMinY(rect) * NSHeight(bounds),
                               NSWidth(rect) * NSWidth(bounds),
                               NSHeight(rect) * NSHeight(bounds)));
    }
}


- (void)drawRect:(NSRect)dirtyRect {
    [super drawRect:dirtyRect];
    
    if (self.image) {
        NSRect bounds = self.bounds;
        drawScaledRect(self.regionOfInterest, bounds, NSColor.greenColor);
        drawScaledRect(self.selectedRegion, bounds, NSColor.redColor);
    }
}


- (BOOL)acceptsFirstMouse:(NSEvent *)event {
    return YES;
}


- (void)mouseDown:(NSEvent *)event {
    selectionStartPoint = [self convertPoint:event.locationInWindow fromView:nil];
    self.selectedRegion = NSZeroRect;
}


- (void)mouseDragged:(NSEvent *)event {
    NSPoint selectionEndPoint = [self convertPoint:event.locationInWindow fromView:nil];
    
    CGFloat minX = fmin(selectionStartPoint.x, selectionEndPoint.x);
    CGFloat maxX = fmax(selectionStartPoint.x, selectionEndPoint.x);
    CGFloat minY = fmin(selectionStartPoint.y, selectionEndPoint.y);
    CGFloat maxY = fmax(selectionStartPoint.y, selectionEndPoint.y);
    
    NSRect bounds = self.bounds;
    NSRect selectionRect = NSIntersectionRect(bounds, NSMakeRect(minX, minY, maxX - minX, maxY - minY));
    self.selectedRegion = NSMakeRect(NSMinX(selectionRect) / NSWidth(bounds),
                                     NSMinY(selectionRect) / NSHeight(bounds),
                                     NSWidth(selectionRect) / NSWidth(bounds),
                                     NSHeight(selectionRect) / NSHeight(bounds));
}


@end

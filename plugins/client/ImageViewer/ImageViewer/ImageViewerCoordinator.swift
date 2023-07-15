//
//  ImageViewerCoordinator.swift
//  ImageViewer
//
//  Created by Christopher Stawarz on 12/7/23.
//  Copyright © 2023 The MWorks Project. All rights reserved.
//

import MWorksCocoa


class ImageViewerCoordinator: ClientPluginCoordinator {
    override var title: String { "Image Viewer" }
    
    @Saved(defaultsKey: "Image Viewer - image data variable",
           workspaceKey: "imageDataVariable",
           receiveEvents: true)
    var imageDataVariable = ""
    
    @Saved(defaultsKey: "Image Viewer - region of interest variable",
           workspaceKey: "regionOfInterestVariable",
           receiveEvents: true)
    var regionOfInterestVariable = ""
    
    weak var imageView: MWImageViewerImageView?
    
    override func receive(event: Event, forSavedProperty property: SavedProperty) {
        switch property {
        case $imageDataVariable:
            imageView?.receivedImageData(event.data.bytesValue)
        case $regionOfInterestVariable:
            receiveRegionOfInterest(event: event)
        default:
            super.receive(event: event, forSavedProperty: property)
        }
    }
    
    func receiveRegionOfInterest(event: Event) {
        guard let imageView else {
            return
        }
        
        var regionOfInterest = NSZeroRect
        
        let roiIsValid = {
            guard let list = event.data.listValue else {
                // Non-list is OK and means no region of intereset
                return true
            }
            
            if list.isEmpty {
                // Empty list is OK and means no region of interest
                return true
            } else if list.count != 4 {
                return false
            }
            
            var values: [Double] = []
            for item in list {
                if let value = item.floatValue, 0.0...1.0 ~= value {
                    values.append(value)
                } else {
                    return false
                }
            }
            
            let minX = values[0]
            let minY = values[1]
            let maxX = values[2]
            let maxY = values[3]
            
            if minX >= maxX || minY >= maxY {
                return false
            }
            
            regionOfInterest = NSMakeRect(minX, minY, maxX - minX, maxY - minY)
            return true
        }()
        if !roiIsValid {
            postError("Region of interest is invalid")
        }
        
        imageView.regionOfInterest = regionOfInterest
    }
    
    func updateROI() {
        guard let imageView else {
            return
        }
        let newROI = imageView.selectedRegion
        if !NSEqualRects(newROI, NSZeroRect) {
            imageView.selectedRegion = NSZeroRect
            let value = Datum([NSMinX(newROI), NSMinY(newROI), NSMaxX(newROI), NSMaxY(newROI)])
            if client?.setValue(value, forTag: regionOfInterestVariable) ?? false {
                imageView.regionOfInterest = newROI
            } else {
                imageView.regionOfInterest = NSZeroRect
            }
        }
    }
    
    func clearROI() {
        guard let imageView else {
            return
        }
        imageView.selectedRegion = NSZeroRect
        if !NSEqualRects(imageView.regionOfInterest, NSZeroRect) {
            client?.setValue(Datum([]), forTag:regionOfInterestVariable)
            imageView.regionOfInterest = NSZeroRect
        }
    }
}

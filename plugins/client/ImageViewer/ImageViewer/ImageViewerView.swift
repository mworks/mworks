//
//  ImageViewerView.swift
//  ImageViewer
//
//  Created by Christopher Stawarz on 7/14/23.
//  Copyright © 2023 The MWorks Project. All rights reserved.
//

import SwiftUI

import MWorksCocoa


struct ImageViewerImageView: NSViewRepresentable {
    @EnvironmentObject private var coordinator: ImageViewerCoordinator
    
    func makeNSView(context: Context) -> MWImageViewerImageView {
        let nsView = MWImageViewerImageView()
        nsView.imageScaling = .scaleProportionallyUpOrDown
        nsView.setContentCompressionResistancePriority(.defaultLow, for: .horizontal)
        nsView.setContentCompressionResistancePriority(.defaultLow, for: .vertical)
        coordinator.imageView = nsView
        return nsView
    }
    
    func updateNSView(_ nsView: MWImageViewerImageView, context: Context) {
    }
}


struct ImageViewerView: View {
    @ObservedObject var coordinator: ImageViewerCoordinator
    
    var body: some View {
        VStack {
            ZStack {
                Text("No Image")
                    .font(.title)
                    .foregroundStyle(.gray)
                ImageViewerImageView()
            }
            .frame(minWidth: 512, minHeight: 288)
            HStack {
                Form {
                    VariableNameField("Image data variable:",
                                      property: coordinator.$imageDataVariable)
                    VariableNameField("Region of interest variable:",
                                      property: coordinator.$regionOfInterestVariable)
                }
                VStack {
                    Button("Update ROI") { coordinator.updateROI() }
                    Button("Clear ROI") { coordinator.clearROI() }
                }
            }
            .padding()
        }
        .environmentObject(coordinator)
    }
}


#Preview("Image Viewer Preview") {
    ImageViewerView(coordinator: ImageViewerCoordinator(client: nil))
}

//
//  ImageViewerViewControllerFactory.swift
//  ImageViewer
//
//  Created by Christopher Stawarz on 12/18/23.
//  Copyright © 2023 The MWorks Project. All rights reserved.
//

import SwiftUI

import MWorksCocoa


class ImageViewerViewControllerFactory: NSObject, ClientPluginViewControllerFactory {
    class func viewController(client: ClientProtocol) -> NSViewController {
        let controller = ClientPluginViewController()
        let coordinator = ImageViewerCoordinator(client: client)
        controller.coordinator = coordinator
        controller.view = NSHostingView(rootView: ImageViewerView(coordinator: coordinator))
        return controller
    }
}

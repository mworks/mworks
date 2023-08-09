//
//  RewardWindowViewControllerFactory.swift
//  MWorksRewardWindow
//
//  Created by Christopher Stawarz on 7/27/23.
//

import SwiftUI

import MWorksCocoa


class RewardWindowViewControllerFactory: NSObject, ClientPluginViewControllerFactory {
    class func viewController(client: ClientProtocol) -> NSViewController {
        let controller = ClientPluginViewController()
        let coordinator = RewardWindowCoordinator(client: client)
        controller.coordinator = coordinator
        controller.view = NSHostingView(rootView: RewardWindowView(coordinator: coordinator))
        return controller
    }
}

//
//  ClientPluginViewController.swift
//  MWorksCocoa
//
//  Created by Christopher Stawarz on 8/9/23.
//

import AppKit


public class ClientPluginViewController: NSViewController, ClientPluginWorkspaceState
{
    public var coordinator: ClientPluginCoordinator? {
        willSet { title = newValue?.title }
    }
    
    public func workspaceState() -> [AnyHashable: Any] {
        coordinator?.workspaceState ?? [:]
    }
    
    public func setWorkspaceState(_ workspaceState: [AnyHashable: Any]) {
        if let coordinator {
            var newState: [String: Any] = [:]
            for (key, value) in workspaceState {
                if let key = key as? String {
                    newState[key] = value
                }
            }
            coordinator.workspaceState = newState
        }
    }
}

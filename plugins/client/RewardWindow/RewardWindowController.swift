//
//  RewardWindowController.swift
//  MWorksRewardWindow
//
//  Created by Christopher Stawarz on 7/27/23.
//

import MWorksCocoa


class RewardWindowController: MWClientPluginViewController {
    
    // Interface Builder doesn't see the "view" property inherited from NSViewController,
    // so we need to define a proxy for it
    @IBOutlet var rewardWindowView: NSView? {
        get { view }
        set { if let newView = newValue { view = newView } }
    }
    
    @objc dynamic var rewardVarName = ""
    @objc dynamic var duration = 0.0
    
    typealias StoredProperty = NSObjectProtocol
    private var rewardVarNameProperty: StoredProperty?
    private var durationProperty: StoredProperty?
    
    override init(client: ClientProtocol) {
        super.init(client: client)
        
        title = "Reward Window"
        
        rewardVarNameProperty = registerStoredProperty(withName: #keyPath(rewardVarName),
                                                       defaultsKey: "Reward Window - var name",
                                                       workspaceKey: "rewardVarName")
        durationProperty = registerStoredProperty(withName: #keyPath(duration),
                                                       defaultsKey: "Reward Window - duration (ms)",
                                                       workspaceKey: "rewardDurationMS")
    }
    
    // FIXME: why is this required?
    required init?(coder: NSCoder) {
        super.init(coder: coder)
    }
    
    override func validateWorkspaceValue(_ value: Any, forStoredProperty property: StoredProperty) -> Bool {
        if property === rewardVarNameProperty {
            return value is String
        } else if property === durationProperty {
            return value is Float
        }
        return super.validateWorkspaceValue(value, forStoredProperty: property)
    }
    
    @IBAction func sendReward(_ sender: Any) {
        // Finish editing in all fields
        if let window = view.window {
            if !window.makeFirstResponder(window) {
                window.endEditing(for: nil)
            }
        }
        
        let rewardVarCode = client?.code(forTag: rewardVarName) ?? -1
        if rewardVarCode < 0 {
            postError("Cannot send reward: Variable \"\(rewardVarName)\" was not found")
        } else {
            client?.setValue(Datum(duration < 0.0 ? 0.0 : duration * 1000.0), forCode: rewardVarCode)
        }
    }
    
}

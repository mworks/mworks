//
//  RewardWindowCoordinator.swift
//  MWorksRewardWindow
//
//  Created by Christopher Stawarz on 8/5/23.
//

import MWorksCocoa


class RewardWindowCoordinator: ClientPluginCoordinator {
    override var title: String { "Reward Window" }
    
    @Stored(defaultsKey: "Reward Window - var name",
            workspaceKey: "rewardVarName")
    var rewardVarName = ""
    
    @Stored(defaultsKey: "Reward Window - duration (ms)",
            workspaceKey: "rewardDurationMS")
    var duration = 0.0
    
    func sendReward() {
        let rewardVarCode = client?.code(forTag: rewardVarName) ?? -1
        if rewardVarCode < 0 {
            postError("Cannot send reward: Variable \"\(rewardVarName)\" was not found")
        } else {
            client?.setValue(Datum(duration < 0.0 ? 0.0 : duration * 1000.0), forCode: rewardVarCode)
        }
    }
}

//
//  RewardWindowView.swift
//  MWorksRewardWindow
//
//  Created by Christopher Stawarz on 8/31/23.
//

import SwiftUI


struct RewardWindowView: View {
    @ObservedObject var coordinator: RewardWindowCoordinator
    
    var body: some View {
        Form {
            TextField("Reward duration (ms):",
                      value: $coordinator.duration,
                      formatter: NumberFormatter())
            TextField("Variable name:",
                      text: $coordinator.rewardVarName)
            Button("Send Reward") { coordinator.sendReward() }
        }
        .frame(minWidth: 300)
        .padding()
    }
}


#Preview("Reward Window Preview") {
    RewardWindowView(coordinator: RewardWindowCoordinator(client: nil))
}

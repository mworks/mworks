//
//  Core.swift
//  MWorksSwift
//
//  Created by Christopher Stawarz on 12/13/23.
//  Copyright © 2023 The MWorks Project. All rights reserved.
//


extension CoreProtocol {
    
    public func registerCallback(withKey key: String,
                                 forCode code: ReservedEventCode,
                                 callback: @escaping EventCallback)
    {
        registerCallback(withKey: key, forCode: code.rawValue, callback: callback)
    }
    
}

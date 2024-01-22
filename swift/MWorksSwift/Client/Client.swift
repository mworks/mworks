//
//  Client.swift
//  MWorksSwift
//
//  Created by Christopher Stawarz on 1/21/24.
//  Copyright © 2024 The MWorks Project. All rights reserved.
//


extension ClientProtocol {
    
    public func value(forCode code: ReservedEventCode) -> Datum? {
        value(forCode: code.rawValue)
    }
    
}

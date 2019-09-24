//
//  Datum.swift
//  MWorksSwift
//
//  Created by Christopher Stawarz on 6/17/19.
//  Copyright © 2019 The MWorks Project. All rights reserved.
//


extension Datum {
    
    public convenience init(_ bool: Bool) {
        self.init(__bool: bool)
    }
    
    public convenience init(_ int: Int) {
        self.init(__int: int)
    }
    
    public convenience init(_ float: Double) {
        self.init(__float: float)
    }
    
    public convenience init(_ bytes: Data) {
        self.init(__bytes: bytes)
    }
    
    public convenience init(_ string: String) {
        self.init(__string: string)
    }
    
    public convenience init(_ list: [Datum]) {
        self.init(__list: list)
    }
    
    public convenience init(_ dict: [Datum: Datum]) {
        self.init(__dict: dict)
    }
    
    public var boolValue: Bool? {
        __numberValue?.boolValue
    }
    
    public var intValue: Int? {
        __numberValue?.intValue
    }
    
    public var floatValue: Double? {
        __numberValue?.doubleValue
    }
    
}

//
//  Datum.swift
//  MWorksSwift
//
//  Created by Christopher Stawarz on 6/17/19.
//  Copyright © 2019 The MWorks Project. All rights reserved.
//


public protocol DatumConvertible {
    var datumValue: Datum { get }
}


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
    
    public convenience init<Value: DatumConvertible>(_ values: [Value]) {
        var list: [Datum] = []
        for value in values {
            list.append(value.datumValue)
        }
        self.init(list)
    }
    
    public convenience init(_ dict: [Datum: Datum]) {
        self.init(__dict: dict)
    }
    
    public convenience init<Key: DatumConvertible, Value: DatumConvertible>(_ items: [Key: Value]) {
        var dict: [Datum: Datum] = [:]
        for (key, value) in items {
            dict[key.datumValue] = value.datumValue
        }
        self.init(dict)
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


extension Bool: DatumConvertible {
    public var datumValue: Datum { Datum(self) }
}


extension Int: DatumConvertible {
    public var datumValue: Datum { Datum(self) }
}


extension Double: DatumConvertible {
    public var datumValue: Datum { Datum(self) }
}


// Not sure why this is necessary, since CGFloat is just a typealias for Double
extension CGFloat: DatumConvertible {
    public var datumValue: Datum { Datum(self) }
}


extension Data: DatumConvertible {
    public var datumValue: Datum { Datum(self) }
}


extension String: DatumConvertible {
    public var datumValue: Datum { Datum(self) }
}

//
//  DatumTests.swift
//  MWorksSwiftTests
//
//  Created by Christopher Stawarz on 6/14/19.
//  Copyright © 2019 The MWorks Project. All rights reserved.
//

import XCTest

import MWorksSwift


class DatumTests: XCTestCase {
    
    func testUndefined() {
        let datum = Datum()
        
        // Data type
        XCTAssertEqual(Datum.DataType.undefined, datum.dataType)
        
        // Values
        XCTAssertNil(datum.boolValue)
        XCTAssertNil(datum.intValue)
        XCTAssertNil(datum.floatValue)
        XCTAssertNil(datum.bytesValue)
        XCTAssertNil(datum.stringValue)
        XCTAssertNil(datum.listValue)
        XCTAssertNil(datum.dictValue)
    }
    
    func testBoolean() {
        let trueDatum = Datum(true)
        let falseDatum = Datum(false)
        
        // Data type
        XCTAssertEqual(Datum.DataType.boolean, trueDatum.dataType)
        XCTAssertEqual(.boolean, falseDatum.dataType)
        
        // Bool value
        XCTAssertEqual(true, trueDatum.boolValue!)
        XCTAssertEqual(false, falseDatum.boolValue!)
        
        // Int value
        XCTAssertEqual(1, trueDatum.intValue!)
        XCTAssertEqual(0, falseDatum.intValue!)
        
        // Float value
        XCTAssertEqual(1.0, trueDatum.floatValue!)
        XCTAssertEqual(0.0, falseDatum.floatValue!)
        
        // Other values
        XCTAssertNil(trueDatum.bytesValue)
        XCTAssertNil(falseDatum.stringValue)
        XCTAssertNil(trueDatum.listValue)
        XCTAssertNil(falseDatum.dictValue)
    }
    
    func testInteger() {
        let zeroDatum = Datum(0)
        let posDatum = Datum(1)
        let negDatum = Datum(-2)
        
        // Data type
        XCTAssertEqual(Datum.DataType.integer, zeroDatum.dataType)
        XCTAssertEqual(.integer, posDatum.dataType)
        XCTAssertEqual(.integer, negDatum.dataType)
        
        // Bool value
        XCTAssertEqual(false, zeroDatum.boolValue!)
        XCTAssertEqual(true, posDatum.boolValue!)
        XCTAssertEqual(true, negDatum.boolValue!)
        
        // Int value
        XCTAssertEqual(0, zeroDatum.intValue!)
        XCTAssertEqual(1, posDatum.intValue!)
        XCTAssertEqual(-2, negDatum.intValue!)
        
        // Float value
        XCTAssertEqual(0.0, zeroDatum.floatValue!)
        XCTAssertEqual(1.0, posDatum.floatValue!)
        XCTAssertEqual(-2.0, negDatum.floatValue!)
        
        // Other values
        XCTAssertNil(zeroDatum.bytesValue)
        XCTAssertNil(posDatum.stringValue)
        XCTAssertNil(negDatum.listValue)
        XCTAssertNil(zeroDatum.dictValue)
    }
    
    func testFloat() {
        let zeroDatum = Datum(0.0)
        let posDatum = Datum(1.5)
        let negDatum = Datum(-2.5)
        
        // Data type
        XCTAssertEqual(Datum.DataType.float, zeroDatum.dataType)
        XCTAssertEqual(.float, posDatum.dataType)
        XCTAssertEqual(.float, negDatum.dataType)
        
        // Bool value
        XCTAssertEqual(false, zeroDatum.boolValue!)
        XCTAssertEqual(true, posDatum.boolValue!)
        XCTAssertEqual(true, negDatum.boolValue!)
        
        // Int value
        XCTAssertEqual(0, zeroDatum.intValue!)
        XCTAssertEqual(1, posDatum.intValue!)
        XCTAssertEqual(-2, negDatum.intValue!)
        
        // Float value
        XCTAssertEqual(0.0, zeroDatum.floatValue!)
        XCTAssertEqual(1.5, posDatum.floatValue!)
        XCTAssertEqual(-2.5, negDatum.floatValue!)
        
        // Other values
        XCTAssertNil(zeroDatum.bytesValue)
        XCTAssertNil(posDatum.stringValue)
        XCTAssertNil(negDatum.listValue)
        XCTAssertNil(zeroDatum.dictValue)
    }
    
    func testBytes() {
        let emptyValue = Data()
        let textValue = Data("abc 123".utf8)
        let dataValue = Data([97, 98, 99, 32, 195, 40, 32, 49, 50, 51])  // "abc \xc3\x28 123"
        
        let emptyDatum = Datum(emptyValue)
        let textDatum = Datum(textValue)
        let dataDatum = Datum(dataValue)
        
        // Data type
        XCTAssertEqual(Datum.DataType.byteString, emptyDatum.dataType)
        XCTAssertEqual(.byteString, textDatum.dataType)
        XCTAssertEqual(.byteString, dataDatum.dataType)
        
        // Bytes value
        XCTAssertEqual(emptyValue, emptyDatum.bytesValue!)
        XCTAssertEqual(textValue, textDatum.bytesValue!)
        XCTAssertEqual(dataValue, dataDatum.bytesValue!)
        
        // String value
        XCTAssertEqual("", emptyDatum.stringValue!)
        XCTAssertEqual("abc 123", textDatum.stringValue!)
        XCTAssertNil(dataDatum.stringValue)
        
        // Other values
        XCTAssertNil(emptyDatum.boolValue)
        XCTAssertNil(textDatum.intValue)
        XCTAssertNil(dataDatum.floatValue)
        XCTAssertNil(emptyDatum.listValue)
        XCTAssertNil(textDatum.dictValue)
    }
    
    func testString() {
        let emptyDatum = Datum("")
        let nonEmptyDatum = Datum("abc 123")
        
        // Data type
        XCTAssertEqual(Datum.DataType.byteString, emptyDatum.dataType)
        XCTAssertEqual(.byteString, nonEmptyDatum.dataType)
        
        // Bytes value
        XCTAssertEqual(Data(), emptyDatum.bytesValue!)
        XCTAssertEqual(Data("abc 123".utf8), nonEmptyDatum.bytesValue!)
        
        // String value
        XCTAssertEqual("", emptyDatum.stringValue!)
        XCTAssertEqual("abc 123", nonEmptyDatum.stringValue!)
        
        // Other values
        XCTAssertNil(emptyDatum.boolValue)
        XCTAssertNil(nonEmptyDatum.intValue)
        XCTAssertNil(emptyDatum.floatValue)
        XCTAssertNil(nonEmptyDatum.listValue)
        XCTAssertNil(emptyDatum.dictValue)
    }
    
    func testList() {
        let emptyDatum = Datum([Datum]())
        let nonEmptyDatum = Datum([Datum(1), Datum(2.0), Datum("three")])
        
        // Data type
        XCTAssertEqual(Datum.DataType.list, emptyDatum.dataType)
        XCTAssertEqual(.list, nonEmptyDatum.dataType)
        
        // List value
        XCTAssertEqual([Datum](), emptyDatum.listValue!)
        XCTAssertEqual([Datum(1), Datum(2.0), Datum("three")], nonEmptyDatum.listValue!)
        XCTAssertNotEqual([Datum(1), Datum(2.0)], nonEmptyDatum.listValue!)
        XCTAssertNotEqual([Datum(1), Datum(2.0), Datum("tree")], nonEmptyDatum.listValue!)
        
        // Other values
        XCTAssertNil(emptyDatum.boolValue)
        XCTAssertNil(nonEmptyDatum.intValue)
        XCTAssertNil(emptyDatum.floatValue)
        XCTAssertNil(nonEmptyDatum.bytesValue)
        XCTAssertNil(emptyDatum.stringValue)
        XCTAssertNil(nonEmptyDatum.dictValue)
    }
    
    func testDict() {
        let emptyDatum = Datum([Datum: Datum]())
        let nonEmptyDatum = Datum([Datum(false): Datum(1), Datum(2.0): Datum("three")])
        
        // Data type
        XCTAssertEqual(Datum.DataType.dictionary, emptyDatum.dataType)
        XCTAssertEqual(.dictionary, nonEmptyDatum.dataType)
        
        // Dict value
        XCTAssertEqual([Datum: Datum](), emptyDatum.dictValue!)
        XCTAssertEqual([Datum(false): Datum(1), Datum(2.0): Datum("three")], nonEmptyDatum.dictValue!)
        XCTAssertNotEqual([Datum(false): Datum(1)], nonEmptyDatum.dictValue!)
        XCTAssertNotEqual([Datum(false): Datum(1), Datum(2.5): Datum("three")], nonEmptyDatum.dictValue!)
        XCTAssertNotEqual([Datum(false): Datum(1), Datum(2.0): Datum("tree")], nonEmptyDatum.dictValue!)
        
        // Other values
        XCTAssertNil(emptyDatum.boolValue)
        XCTAssertNil(nonEmptyDatum.intValue)
        XCTAssertNil(emptyDatum.floatValue)
        XCTAssertNil(nonEmptyDatum.bytesValue)
        XCTAssertNil(emptyDatum.stringValue)
        XCTAssertNil(nonEmptyDatum.listValue)
    }
    
    func testIndexedSubscript() {
        // Non-list, non-dict
        XCTAssertNil(Datum(1)[2])
        
        // Empty list
        XCTAssertNil(Datum([Datum]())[2])
        
        // Non-empty list
        do {
            let listDatum = Datum([Datum(1.5), Datum("foo"), Datum("bar")])
            
            // Invalid indexes
            XCTAssertNil(listDatum[-1])
            XCTAssertNil(listDatum[3])
            
            // Valid indexes
            XCTAssertEqual(Datum(1.5), listDatum[0]!)
            XCTAssertEqual(Datum("foo"), listDatum[1]!)
            XCTAssertEqual(Datum("bar"), listDatum[2]!)
        }
        
        // Empty dict
        XCTAssertNil(Datum([Datum: Datum]())[2])
        
        // Non-empty dict
        do {
            let dictDatum = Datum([Datum(2): Datum("foo"), Datum(-3): Datum("bar")])
            
            // Invalid index
            XCTAssertNil(dictDatum[1])
            
            // Valid indexes
            XCTAssertEqual(Datum("foo"), dictDatum[2]!)
            XCTAssertEqual(Datum("bar"), dictDatum[-3]!)
        }
    }
    
    func testKeyedSubscript() {
        // Non-dict
        XCTAssertNil(Datum(1)["foo"])
        
        // Empty dict
        XCTAssertNil(Datum([Datum: Datum]())["foo"])
        
        // Non-empty dict
        do {
            let dictDatum = Datum([Datum("foo"): Datum(2), Datum("bar"): Datum(3)])
            
            // Invalid key
            XCTAssertNil(dictDatum["blah"])
            
            // Valid keys
            XCTAssertEqual(Datum(2), dictDatum["foo"]!)
            XCTAssertEqual(Datum(3), dictDatum["bar"]!)
        }
    }
    
}

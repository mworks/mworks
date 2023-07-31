//
//  ClientPluginCoordinator.swift
//  MWorksCocoa
//
//  Created by Christopher Stawarz on 7/31/23.
//

import Combine

import MWorksSwift


private protocol StoredProperty {
    var objectWillChange: ObservableObjectPublisher? { get set }
    var workspaceKey: String { get }
    var workspaceValue: Any { get set }
}


open class ClientPluginCoordinator: ObservableObject {
    @propertyWrapper
    public class Stored<Value>: StoredProperty where Value: Codable & Equatable {
        public typealias Validator = (Value) -> Bool
        
        private let subject: CurrentValueSubject<Value, Never>
        private let validator: Validator?
        private let defaultsKey: String
        
        fileprivate var objectWillChange: ObservableObjectPublisher?
        fileprivate let workspaceKey: String
        fileprivate var workspaceValue: Any {
            get { wrappedValue }
            set {
                if let newValue = newValue as? Value {
                    wrappedValue = newValue
                }
            }
        }
        
        public var wrappedValue: Value {
            get { subject.value }
            set {
                if newValue != subject.value, validator?(newValue) ?? true {
                    objectWillChange?.send()
                    subject.value = newValue
                    UserDefaults.standard.set(newValue, forKey: defaultsKey)
                }
            }
        }
        public var projectedValue: some Publisher<Value, Never> { subject }
        
        public init(wrappedValue: Value, _ validator: Validator?, defaultsKey: String, workspaceKey: String) {
            var initialValue = wrappedValue
            if let defaultValue = UserDefaults.standard.object(forKey: defaultsKey) as? Value {
                initialValue = defaultValue
            }
            self.subject = CurrentValueSubject(initialValue)
            self.validator = validator
            self.defaultsKey = defaultsKey
            self.workspaceKey = workspaceKey
        }
        
        public convenience init(wrappedValue: Value, defaultsKey: String, workspaceKey: String) {
            self.init(wrappedValue: wrappedValue, nil, defaultsKey: defaultsKey, workspaceKey: workspaceKey)
        }
    }
    
    public private(set) weak var client: ClientProtocol?
    open var title: String { "" }
    
    private var storedProperties: [StoredProperty] = []
    
    public init(client: ClientProtocol?) {
        self.client = client
        
        var reflection: Mirror? = Mirror(reflecting: self)
        while let currentClass = reflection {
            for (_, property) in currentClass.children {
                if var property = property as? StoredProperty {
                    property.objectWillChange = objectWillChange
                    storedProperties.append(property)
                }
            }
            reflection = currentClass.superclassMirror
        }
    }
    
    public var workspaceState: [String: Any] {
        get {
            var state: [String: Any] = [:]
            for property in storedProperties {
                state[property.workspaceKey] = property.workspaceValue
            }
            return state
        }
        set(newState) {
            for var property in storedProperties {
                if let newValue = newState[property.workspaceKey] {
                    property.workspaceValue = newValue
                }
            }
        }
    }
    
    public func postMessage(_ message: String) {
        client?.logMessage("\(title): \(message)")
    }
    
    public func postWarning(_ warning: String) {
        client?.logWarning("\(title): \(warning)")
    }
    
    public func postError(_ error: String) {
        client?.logError("\(title): \(error)")
    }
}

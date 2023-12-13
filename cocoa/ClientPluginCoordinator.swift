//
//  ClientPluginCoordinator.swift
//  MWorksCocoa
//
//  Created by Christopher Stawarz on 7/31/23.
//

import Combine

import MWorksSwift


private protocol StoredPropertyPrivate: AnyObject {
    var name: String? { get set }
    var objectWillChange: ObservableObjectPublisher? { get set }
    var workspaceKey: String { get }
    var workspaceValue: Any { get set }
}


open class ClientPluginCoordinator: ObservableObject {
    public class StoredProperty {
        public static func ~= (lhs: StoredProperty, rhs: StoredProperty) -> Bool {
            return lhs === rhs
        }
    }
    
    @propertyWrapper
    public class Stored<Value>: StoredProperty, StoredPropertyPrivate where Value: Codable & Equatable {
        public typealias Validator = (Value) -> Bool
        
        private let subject: CurrentValueSubject<Value, Never>
        private let validator: Validator?
        private let defaultsKey: String
        
        fileprivate var name: String?
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
        
        fileprivate let receiveEvents: Bool
        
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
        public var projectedValue: Stored<Value> { self }
        public var publisher: some Publisher<Value, Never> { subject }
        
        public init(wrappedValue: Value,
                    _ validator: Validator?,
                    defaultsKey: String,
                    workspaceKey: String,
                    receiveEvents: Bool = false)
        {
            let initialValue = (UserDefaults.standard.object(forKey: defaultsKey) as? Value) ?? wrappedValue
            self.subject = CurrentValueSubject(initialValue)
            self.validator = validator
            self.defaultsKey = defaultsKey
            self.workspaceKey = workspaceKey
            self.receiveEvents = receiveEvents
        }
        
        public convenience init(wrappedValue: Value,
                                defaultsKey: String,
                                workspaceKey: String,
                                receiveEvents: Bool = false)
        {
            self.init(wrappedValue: wrappedValue,
                      nil,
                      defaultsKey: defaultsKey,
                      workspaceKey: workspaceKey,
                      receiveEvents: receiveEvents)
        }
    }
    
    public private(set) weak var client: ClientProtocol?
    open var title: String { "" }
    
    private let uuid = UUID()
    private var storedProperties: [StoredPropertyPrivate] = []
    private var eventReceivers: [Stored<String>] = []
    private var haveEventReceivers: Bool { !eventReceivers.isEmpty }
    private var cancellables: [AnyCancellable] = []
    
    private func eventCallbackKey(label: String) -> String {
        "\(type(of:self)) \(uuid.uuidString) \(label) callback key"
    }
    
    private func eventCallbackKey(property: StoredPropertyPrivate) -> String {
        eventCallbackKey(label: "\(property.name ?? property.workspaceKey) property")
    }
    
    private func codecEventCallbackKey() -> String {
        eventCallbackKey(label: "codec")
    }
    
    private func registerEventCallback(forTag tag: String, property: Stored<String>) {
        guard let client else {
            return
        }
        let key = eventCallbackKey(property: property)
        client.unregisterCallbacks(withKey: key)
        if !tag.isEmpty {
            let code = client.code(forTag: tag)
            if code < 0 {
                postWarning("Can't find variable \"\(tag)\"")
            } else {
                client.registerCallback(withKey: key, forCode: code) {
                    [weak self, unowned property /*self owns property*/] event in
                    self?.receive(event: event, forStoredProperty: property)
                }
            }
        }
    }
    
    private func registerEventCallbacks() {
        for property in eventReceivers {
            registerEventCallback(forTag: property.wrappedValue, property: property)
        }
    }
    
    public init(client: ClientProtocol?) {
        self.client = client
        
        var reflection: Mirror? = Mirror(reflecting: self)
        while let currentClass = reflection {
            for (name, property) in currentClass.children {
                if let property = property as? StoredPropertyPrivate {
                    property.name = name
                    property.objectWillChange = objectWillChange
                    storedProperties.append(property)
                    if let property = property as? Stored<String>, property.receiveEvents {
                        eventReceivers.append(property)
                        cancellables.append(property.publisher.sink {
                            [unowned self, unowned property] value in
                            self.registerEventCallback(forTag: value, property: property)
                        })
                    }
                }
            }
            reflection = currentClass.superclassMirror
        }
        
        if haveEventReceivers {
            registerEventCallbacks()
            client?.registerCallback(withKey: codecEventCallbackKey(),
                                     forCode: ReservedEventCode.codec,
                                     callback: { [weak self] event in self?.registerEventCallbacks() })
        }
    }
    
    deinit {
        if haveEventReceivers {
            client?.unregisterCallbacks(withKey: codecEventCallbackKey())
            for property in eventReceivers {
                client?.unregisterCallbacks(withKey: eventCallbackKey(property: property))
            }
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
            for property in storedProperties {
                if let newValue = newState[property.workspaceKey] {
                    property.workspaceValue = newValue
                }
            }
        }
    }
    
    open func receive(event: Event, forStoredProperty property: StoredProperty) {
        // Default implementation does nothing
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

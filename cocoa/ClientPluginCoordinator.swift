//
//  ClientPluginCoordinator.swift
//  MWorksCocoa
//
//  Created by Christopher Stawarz on 7/31/23.
//

import Combine

import MWorksSwift


private protocol SavedPropertyPrivate: AnyObject {
    var name: String? { get set }
    var objectWillChange: ObservableObjectPublisher? { get set }
    var workspaceKey: String { get }
    var workspaceValue: Any { get set }
}


open class ClientPluginCoordinator: ObservableObject {
    public class SavedProperty {
        public static func ~= (lhs: SavedProperty, rhs: SavedProperty) -> Bool {
            return lhs === rhs
        }
    }
    
    @propertyWrapper
    public class Saved<Value>: SavedProperty, SavedPropertyPrivate where Value: Codable & Equatable {
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
        public var projectedValue: Saved<Value> { self }
        public var publisher: some Publisher<Value, Never> { subject }
        
        public init(wrappedValue: Value,
                    defaultsKey: String,
                    workspaceKey: String,
                    receiveEvents: Bool = false,
                    validator: Validator? = nil)
        {
            let initialValue = (UserDefaults.standard.object(forKey: defaultsKey) as? Value) ?? wrappedValue
            self.subject = CurrentValueSubject(initialValue)
            self.defaultsKey = defaultsKey
            self.workspaceKey = workspaceKey
            self.receiveEvents = receiveEvents
            self.validator = validator
        }
    }
    
    public private(set) weak var client: ClientProtocol?
    open var title: String { "" }
    
    private let uuid = UUID()
    private var savedProperties: [SavedPropertyPrivate] = []
    private var eventReceivers: [Saved<String>] = []
    private var haveEventReceivers: Bool { !eventReceivers.isEmpty }
    private var cancellables: [AnyCancellable] = []
    
    private func eventCallbackKey(label: String) -> String {
        "\(type(of:self)) \(uuid.uuidString) \(label) callback key"
    }
    
    private func eventCallbackKey(property: SavedPropertyPrivate) -> String {
        eventCallbackKey(label: "\(property.name ?? property.workspaceKey) property")
    }
    
    private func codecEventCallbackKey() -> String {
        eventCallbackKey(label: "codec")
    }
    
    private func registerEventCallback(forTag tag: String, property: Saved<String>) {
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
                    self?.receive(event: event, forSavedProperty: property)
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
                if let property = property as? SavedPropertyPrivate {
                    property.name = name
                    property.objectWillChange = objectWillChange
                    savedProperties.append(property)
                    if let property = property as? Saved<String>, property.receiveEvents {
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
            for property in savedProperties {
                state[property.workspaceKey] = property.workspaceValue
            }
            return state
        }
        set(newState) {
            for property in savedProperties {
                if let newValue = newState[property.workspaceKey] {
                    property.workspaceValue = newValue
                }
            }
        }
    }
    
    open func receive(event: Event, forSavedProperty property: SavedProperty) {
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

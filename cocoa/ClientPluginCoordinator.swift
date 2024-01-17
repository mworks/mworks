//
//  ClientPluginCoordinator.swift
//  MWorksCocoa
//
//  Created by Christopher Stawarz on 7/31/23.
//

//
// We need to use @preconcurrency to suppress a warning that ObservableObjectPublisher isn't Sendable.
//
// Based on the discussion at https://forums.swift.org/t/61802, it seems like declaring objectWillChange
// nonisolated should resolve this, but as of Xcode 15.2, it does not.  Is that a compiler bug?
//
@preconcurrency import Combine

import MWorksSwift


@MainActor
open class ClientPluginCoordinator: ObservableObject {
    public private(set) weak var client: ClientProtocol?
    open var title: String { "" }
    
    public let objectWillChange = ObservableObjectPublisher()
    
    public private(set) var variableNames: Set<String> = []
    
    private var savedProperties: [SavedPropertyPrivate] = []
    private var eventReceivers: [String: Saved<String>] = [:]
    private var previousCodec = Datum()
    
    public init(client: ClientProtocol?) {
        self.client = client
        
        let uuid = UUID().uuidString
        func eventCallbackKey(label: String) -> String {
            "\(type(of: self)) \(uuid) \(label) callback key"
        }
        
        var reflection: Mirror? = Mirror(reflecting: self)
        while let currentClass = reflection {
            for (name, property) in currentClass.children {
                if let property = property as? SavedPropertyPrivate {
                    property.coordinator = self
                    savedProperties.append(property)
                    if let name, let property = property as? Saved<String>, property.receiveEvents {
                        let key = eventCallbackKey(label: "\(name) property")
                        eventReceivers[key] = property
                        property.valueDidChange = {
                            // Since property is @MainActor, and valueDidChange is called on
                            // the actor on which the property is modified, this will always
                            // execute on MainActor
                            [weak self, unowned property /*self owns property*/] value in
                            self?.registerEventCallback(withKey: key, forTag: value, property: property)
                        }
                    }
                }
            }
            reflection = currentClass.superclassMirror
        }
        
        if let client {
            if let codec = client.value(forCode: ReservedEventCode.codec) {
                receive(codec: codec)
            }
            client.registerCallback(withKey: eventCallbackKey(label: "codec"),
                                    forCode: ReservedEventCode.codec) {
                [weak self] event in
                if let self {
                    Task { @MainActor in self.receive(codec: event.data) }
                }
            }
        }
    }
    
    //
    // TODO: It would be nice to unregister the callbacks in deinit, but doing so generates
    // warnings that client isn't sendable, and it's not clear that we can convince the compiler
    // that it is.  In any case, it probably doesn't matter, because any ClientPluginCoordinator
    // instance should persist until MWClient quits.
    //
    /*
    deinit {
        client?.unregisterCallbacks(withKey: codecEventCallbackKey)
        for (key, _) in eventReceivers {
            client?.unregisterCallbacks(withKey: key)
        }
    }
     */
    
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
    
    private func registerEventCallback(withKey key: String, forTag tag: String, property: Saved<String>) {
        guard let client else {
            return
        }
        client.unregisterCallbacks(withKey: key)
        if !tag.isEmpty {
            let code = client.code(forTag: tag)
            if code >= 0 {
                client.registerCallback(withKey: key, forCode: code) {
                    [weak self, unowned property /*self owns property*/] event in
                    if let self {
                        Task { @MainActor in self.receive(event: event, forSavedProperty: property) }
                    }
                }
            }
        }
    }
    
    private func receive(codec: Datum) {
        if previousCodec == codec {
            // Codec hasn't changed, so there's nothing to do
            return
        }
        
        var newVariableNames = Set<String>()
        if let codec = codec.dictValue {
            for (_, info) in codec {
                if let tagname = info["tagname"]?.stringValue {
                    newVariableNames.insert(tagname)
                }
            }
        }
        objectWillChange.send()
        variableNames = newVariableNames
        
        for (key, property) in eventReceivers {
            registerEventCallback(withKey: key, forTag: property.wrappedValue, property: property)
        }
        
        previousCodec = codec
    }
}


extension ClientPluginCoordinator {
    @MainActor
    public class SavedProperty {
        public static func ~= (lhs: SavedProperty, rhs: SavedProperty) -> Bool {
            return lhs === rhs
        }
    }
    
    @propertyWrapper
    public class Saved<Value>: SavedProperty, SavedPropertyPrivate where Value: Codable & Equatable {
        public typealias Validator = (Value) -> Bool
        
        private var value: Value
        private let validator: Validator?
        private let defaultsKey: String
        
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
        fileprivate var valueDidChange: ((Value) -> Void)?
        
        public fileprivate(set) weak var coordinator: ClientPluginCoordinator?
        
        public var wrappedValue: Value {
            get { value }
            set {
                if newValue != value, validator?(newValue) ?? true {
                    coordinator?.objectWillChange.send()
                    value = newValue
                    valueDidChange?(newValue)
                    UserDefaults.standard.set(newValue, forKey: defaultsKey)
                }
            }
        }
        public var projectedValue: Saved<Value> { self }
        
        public init(wrappedValue: Value,
                    defaultsKey: String,
                    workspaceKey: String,
                    receiveEvents: Bool = false,
                    validator: Validator? = nil)
        {
            self.value = (UserDefaults.standard.object(forKey: defaultsKey) as? Value) ?? wrappedValue
            self.defaultsKey = defaultsKey
            self.workspaceKey = workspaceKey
            self.receiveEvents = receiveEvents
            self.validator = validator
        }
    }
}


@MainActor
private protocol SavedPropertyPrivate: AnyObject {
    var workspaceKey: String { get }
    var workspaceValue: Any { get set }
    var coordinator: ClientPluginCoordinator? { get set }
}

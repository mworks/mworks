//
//  VariableNameField.swift
//  MWorksCocoa
//
//  Created by Christopher Stawarz on 1/30/24.
//

import SwiftUI


public struct VariableNameField: View {
    public typealias SavedProperty = ClientPluginCoordinator.Saved<String>
    
    public let title: String
    public let property: SavedProperty
    
    @ObservedObject private var coordinator: ClientPluginCoordinator
    
    public init(_ title: String, property: SavedProperty) {
        self.title = title
        self.property = property
        coordinator = property.coordinator!
    }
    
    public var body: some View {
        let inputField = InputField(property: property)
            .overlay(alignment: .trailing) {
                let isValid = coordinator.variableNames.contains(property.wrappedValue)
                Image(systemName: "\(isValid ? "checkmark" : "xmark").circle.fill")
                    .foregroundStyle(isValid ? .green : .red)
                    .padding(.trailing, 3)
                    .help("Variable name is " + (isValid ? "valid" : "not valid"))
            }
        
        if #available(macOS 13, *) {
            LabeledContent(title) {
                inputField
            }
        } else {
            HStack {
                Text(title)
                inputField
            }
        }
    }
}


extension VariableNameField {
    private struct InputField: NSViewRepresentable {
        let property: SavedProperty
        
        @ObservedObject private var clientPluginCoordinator: ClientPluginCoordinator
        
        init(property: SavedProperty) {
            self.property = property
            clientPluginCoordinator = property.coordinator!
        }
        
        func makeNSView(context: Context) -> NSTextField {
            let nsView = NSTextField(string: property.wrappedValue)
            nsView.delegate = context.coordinator
            return nsView
        }
        
        func updateNSView(_ nsView: NSTextField, context: Context) {
            nsView.currentEditor()?.string = property.wrappedValue
        }
        
        func makeCoordinator() -> Coordinator {
            Coordinator(property: property)
        }
        
        class Coordinator: NSObject, NSTextFieldDelegate {
            let property: SavedProperty
            
            init(property: SavedProperty) {
                self.property = property
            }
            
            func controlTextDidChange(_ obj: Notification) {
                if let textView = obj.userInfo?["NSFieldEditor"] as? NSTextView {
                    let currentText = textView.string
                    if (currentText != property.wrappedValue) {
                        property.wrappedValue = currentText
                        if !currentText.isEmpty {
                            textView.complete(nil)
                        }
                    }
                }
            }
            
            func control(
                _ control: NSControl,
                textView: NSTextView,
                completions words: [String],
                forPartialWordRange charRange: NSRange,
                indexOfSelectedItem index: UnsafeMutablePointer<Int>) -> [String]
            {
                let currentText = textView.string
                let matches = property.coordinator!.variableNames.filter { $0.starts(with: currentText) }
                if matches.isEmpty || (matches.count == 1 && matches.contains(currentText)) {
                    // If there are no matches, or if currentText itself is the only match,
                    // return no suggested completions
                    return []
                }
                // Don't select any suggestion.  This allows the user to ignore all suggestions
                // and keep typing.
                index.pointee = -1
                return Array(matches).sorted()
            }
        }
    }
}

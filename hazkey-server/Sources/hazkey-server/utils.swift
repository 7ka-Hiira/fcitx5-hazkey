import Foundation

// log for debugging
public func debugLog(
    _ items: Any?, function: String = #function
) {
    #if DEBUG
        if let items = items {
            NSLog("\(function) : \(items)")
        } else {
            NSLog("\(function)")
        }
    #endif
}

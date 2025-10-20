import Dispatch
import Foundation
import KanaKanjiConverterModule

do {
    let server = HazkeyServer()

    try server.start()
} catch {
    NSLog("Failed to start server: \(error)")
    exit(1)
}

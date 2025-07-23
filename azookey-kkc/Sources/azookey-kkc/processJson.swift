import Foundation

enum KkcFunction: Decodable {
    case set_config
    case set_left_context
    case create_composing_text_instance
}

struct QueryData: Decodable {
    let function: KkcFunction
    let props: String?
}

func processJson(jsonString: String) {
    if let jsonData = jsonString.data(using: .utf8) {
        do {
            let parsed = try JSONDecoder().decode(QueryData.self, from: jsonData)
            print(parsed)
        } catch {
            NSLog("failed to perse JSON: \(error)")
            NSLog("Input: \(jsonString)")
        }
    }
}

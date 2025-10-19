import Foundation
import SwiftProtobuf

@MainActor
func processProto(data: Data) -> Data {
    let query: Hazkey_RequestEnvelope
    let response: Hazkey_ResponseEnvelope
    do {
        query = try Hazkey_RequestEnvelope(serializedBytes: data)
    } catch {
        NSLog("Failed to parse protobuf: \(error)")
        response = Hazkey_ResponseEnvelope.with {
            $0.status = .failed
            $0.errorMessage = "Failed to parse protobuf: \(error)"
        }
        return serializeResult(unserialized: response)
    }

    switch query.payload {
    case .setContext(let req):
        response = setContext(surroundingText: req.context, anchorIndex: Int(req.anchor))
    case .newComposingText:
        response = createComposingTextInstanse()
    case .inputChar(let req):
        response = inputChar(inputString: req.text)
    case .modifierEvent(let req):
        response = processModifierEvent(modifier: req.modType, event: req.eventType)
    case .deleteLeft:
        response = deleteLeft()
    case .deleteRight:
        response = deleteRight()
    case .prefixComplete(let req):
        response = completePrefix(candidateIndex: Int(req.index))
    case .moveCursor(let req):
        response = moveCursor(offset: Int(req.offset))
    case .getHiraganaWithCursor:
        response = getHiraganaWithCursor()
    case .getComposingString(let req):
        response = getComposingString(charType: req.charType, currentPreedit: req.currentPreedit)
    case .getCandidates(let req):
        response = getCandidates(is_suggest: req.isSuggest)
    case .getCurrentInputMode:
        response = getCurrentInputMode()
    case .getConfig:
        response = getCurrentConfig()
    case .setConfig(let req):
        response = setCurrentConfig(req.fileHashes, req.profiles)
    case .getDefaultProfile, .clearAllHistory_p:
        NSLog("Unimplemented: getDefaultProfile")
        response = Hazkey_ResponseEnvelope.with {
            $0.status = .failed
            $0.errorMessage = "Unimplemented: getDefaultProfile"
        }
    case .none:
        NSLog("Payload not specified")
        response = Hazkey_ResponseEnvelope.with {
            $0.status = .failed
            $0.errorMessage = "Payload not specified"
        }
    }
    return serializeResult(unserialized: response)
}

func serializeResult(unserialized: Hazkey_ResponseEnvelope) -> Data {
    do {
        let serialized = try unserialized.serializedData()
        return serialized
    } catch {
        NSLog("Failed to serialize response message: \(unserialized)")
        return Data()
    }
}

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
  case .newComposingText(_):
    response = createComposingTextInstanse()
  case .inputChar(let req):
    response = inputChar(inputString: req.text, isDirect: req.isDirect)
  case .deleteLeft(_):
    response = deleteLeft()
  case .deleteRight(_):
    response = deleteRight()
  case .prefixComplete(let req):
    response = completePrefix(candidateIndex: Int(req.index))
  case .moveCursor(let req):
    response = moveCursor(offset: Int(req.offset))
  case .getHiraganaWithCursor(_):
    response = getHiraganaWithCursor()
  case .getComposingString(let req):
    response = getComposingString(charType: req.charType, currentPreedit: req.currentPreedit)
  case .getCandidates(let req):
    response = getCandidates(isPredictMode: req.isPredictMode, nBest: Int(req.nBest))
  case .setConfig(let req):
    response = setConfig(req.hash, req.tableOperations, req.config)
  default:
    NSLog("Unimplemented command")
    response = Hazkey_ResponseEnvelope.with {
      $0.status = .failed
      $0.errorMessage = "Unimplemented command: \(query)"
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

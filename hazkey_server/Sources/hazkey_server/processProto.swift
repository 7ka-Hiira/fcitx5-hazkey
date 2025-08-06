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
  case .setConfig:
    let props = query.setConfig
    response = setConfig(
      zenzaiEnabled: props.zenzaiEnabled,
      zenzaiInferLimit: Int(props.zenzaiInferLimit),
      numberFullwidth: Int(props.numberFullwidth),
      symbolFullwidth: Int(props.symbolFullwidth),
      periodStyleIndex: Int(props.periodStyle),
      commaStyleIndex: Int(props.commaStyle),
      spaceFullwidth: Int(props.spaceFullwidth),
      tenCombining: Int(props.tenCombining),
      profileText: props.profileText
    )
  case .setContext:
    let props = query.setContext
    response = setContext(surroundingText: props.context, anchorIndex: Int(props.anchor))
  case .newComposingText:
    response = createComposingTextInstanse()
  case .inputChar:
    let props = query.inputChar
    response = inputChar(inputString: props.text, isDirect: props.isDirect)
  case .deleteLeft:
    response = deleteLeft()
  case .deleteRight:
    response = deleteRight()
  case .prefixComplete:
    let props = query.prefixComplete
    response = completePrefix(candidateIndex: Int(props.index))
  case .moveCursor:
    let props = query.moveCursor
    response = moveCursor(offset: Int(props.offset))
  case .getHiraganaWithCursor:
    response = getHiraganaWithCursor()
  case .getComposingString:
    let props = query.getComposingString
    response = getComposingString(charType: props.charType, currentPreedit: props.currentPreedit)
  case .getCandidates:
    let props = query.getCandidates
    response = getCandidates(isPredictMode: props.isPredictMode, nBest: Int(props.nBest))
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

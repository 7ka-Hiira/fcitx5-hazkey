import Foundation
import SwiftProtobuf

@MainActor
func processProto(data: Data) -> Data {
  let query: Hazkey_Commands_QueryData
  let response: Hazkey_Commands_ResultData
  do {
    query = try Hazkey_Commands_QueryData(serializedBytes: data)
  } catch {
    NSLog("Failed to parse protobuf: \(error)")
    response = Hazkey_Commands_ResultData.with {
      $0.status = .failed
      $0.errorMessage = "Failed to parse protobuf: \(error)"
    }
    return serializeResult(unserialized: response)
  }

  switch query.function {
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
  case .setLeftContext:
    let props = query.setLeftContext
    response = setLeftContext(surroundingText: props.context, anchorIndex: Int(props.anchor))
  case .createComposingTextInstance:
    response = createComposingTextInstanse()
  case .inputText:
    let props = query.inputText
    response = inputText(inputString: props.text, isDirect: props.isDirect)
  case .deleteLeft:
    response = deleteLeft()
  case .deleteRight:
    response = deleteRight()
  case .completePrefix:
    let props = query.completePrefix
    response = completePrefix(candidateIndex: Int(props.index))
  case .moveCursor:
    let props = query.moveCursor
    response = moveCursor(offset: Int(props.offset))
  case .getHiraganaWithCursor:
    response = getHiraganaWithCursor()
  case .getComposingString:
    let props = query.getComposingString
    response = getComposingString(charType: props.charType)
  case .getCandidates:
    let props = query.getCandidates
    response = getCandidates(isPredictMode: props.isPredictMode, nBest: Int(props.nBest))
  case .UNRECOGNIZED(_):
    NSLog("Unimplemented function")
    response = Hazkey_Commands_ResultData.with {
      $0.status = .failed
      $0.errorMessage = "Unimplemented function: \(query.function.rawValue)"
    }
  }
  return serializeResult(unserialized: response)
}

func serializeResult(unserialized: Hazkey_Commands_ResultData) -> Data {
  do {
    let serialized = try unserialized.serializedData()
    return serialized
  } catch {
    NSLog("Failed to serialize response message: \(unserialized)")
    return Data()
  }
}

import Foundation
import SwiftProtobuf

@MainActor
func processProto(data: Data) -> Data {
  do {
    let query = try Hazkey_QueryData(serializedBytes: data)
    switch query.function {
    case .setConfig:
      let props = query.setConfig
      setConfig(
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
      setLeftContext(surroundingText: props.context, anchorIndex: Int(props.anchor))
    case .createComposingTextInstance:
      createComposingTextInstanse()
    case .inputText:
      let props = query.inputText
      let response = inputText(inputString: props.text, isDirect: props.isDirect)
      let serialized = try response.serializedData()
      return serialized
    case .deleteLeft:
      deleteLeft()
    case .deleteRight:
      deleteRight()
    case .completePrefix:
      let props = query.completePrefix
      completePrefix(candidateIndex: Int(props.index))
    case .moveCursor:
      let props = query.moveCursor
      moveCursor(offset: Int(props.offset))
    case .getHiraganaWithCursor:
      let result = getHiraganaWithCursor()
      let response = Hazkey_SimpleResult.with {
        $0.status = Hazkey_StatusCode.success
        $0.result = result
      }
      let serialized = try response.serializedData()
      return serialized
    case .getComposingString:
      let props = query.getComposingString
      let response = getComposingString(charType: props.charType)
      let serialized = try response.serializedData()
      return serialized
    case .getCandidates:
      let props = query.getCandidates
      let response = getCandidates(isPredictMode: props.isPredictMode, nBest: Int(props.nBest))
      let serialized = try response.serializedData()
      return serialized
    case .UNRECOGNIZED(_):
      NSLog("Unimplemented function")
      let response = Hazkey_Status.with {
        $0.status = .failed
      }
      let serialized = try response.serializedData()
      return serialized
    }
    let response = Hazkey_Status.with { $0.status = .success }
    let serialized = try response.serializedData()
    return serialized
  } catch {
    NSLog("failed to parse protobuf: \(error)")

    let response = Hazkey_Status.with { $0.status = .failed }
    let serialized = try! response.serializedData()
    return serialized
  }
}

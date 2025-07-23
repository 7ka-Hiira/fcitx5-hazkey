import Foundation

enum KkcApi: String, Decodable {
  case set_config
  case set_left_context
  case create_composing_text_instance
  case input_text
  case delete_left
  case delete_right
  case complete_prefix
  case move_cursor
  case get_hiragana_with_cursor
  case get_composing_string
  case get_candidates
}

struct AnyDecodable: Decodable {
  let value: Any

  init(from decoder: Decoder) throws {
    let container = try decoder.singleValueContainer()
    if let intValue = try? container.decode(Int.self) {
      value = intValue
    } else if let doubleValue = try? container.decode(Double.self) {
      value = doubleValue
    } else if let boolValue = try? container.decode(Bool.self) {
      value = boolValue
    } else if let stringValue = try? container.decode(String.self) {
      value = stringValue
    } else if let arrayValue = try? container.decode([AnyDecodable].self) {
      value = arrayValue.map { $0.value }
    } else if let dictValue = try? container.decode([String: AnyDecodable].self) {
      value = dictValue.mapValues { $0.value }
    } else {
      throw DecodingError.dataCorruptedError(in: container, debugDescription: "Cannot decode value")
    }
  }
}

struct SetConfigProps: Decodable {
  let zenzai_enabled: Bool
  let zenzai_infer_limit: Int
  let number_fullwidth: Int
  let symbol_fullwidth: Int
  let period_style: Int
  let comma_style: Int
  let space_fullwidth: Int
  let ten_combining: Int
  let profile_text: String
}

struct SimpleResult: Encodable {
  let result: String
}

struct EmptyProps: Decodable {}

struct SetLeftContextProps: Decodable {
  let context: String
  let anchor: Int
}

struct InputTextProps: Decodable {
  let text: String
  let is_direct: Bool
}

struct MoveCursorProps: Decodable {
  let offset: Int
}

struct GetComposingStringProps: Decodable {
  let char_type: CharType
}

struct QueryData: Decodable {
  let function: KkcApi
  let props: [String: AnyDecodable]?
}

@MainActor func processJson(jsonString: String) -> String {
  print(jsonString)
  guard let jsonData = jsonString.data(using: .utf8) else {
    NSLog("Invalid UTF-8 data for input: \(jsonString)")
    return "{}"
  }
  do {
    let query = try JSONDecoder().decode(QueryData.self, from: jsonData)
    let propsDict = query.props?.mapValues { $0.value } ?? [:]
    let propsData = try JSONSerialization.data(withJSONObject: propsDict, options: [])

    switch query.function {
    case .set_config:
      let props = try JSONDecoder().decode(SetConfigProps.self, from: propsData)
      setConfig(
        zenzaiEnabled: props.zenzai_enabled, zenzaiInferLimit: props.zenzai_infer_limit,
        numberFullwidth: props.number_fullwidth, symbolFullwidth: props.symbol_fullwidth,
        periodStyleIndex: props.period_style, commaStyleIndex: props.comma_style,
        spaceFullwidth: props.space_fullwidth, tenCombining: props.ten_combining,
        profileText: props.profile_text)
    case .set_left_context:
      let props = try JSONDecoder().decode(SetLeftContextProps.self, from: propsData)
      setLeftContext(surroundingText: props.context, anchorIndex: props.anchor)
    case .create_composing_text_instance:
      createComposingTextInstanse()
    case .input_text:
      let props = try JSONDecoder().decode(InputTextProps.self, from: propsData)
      inputText(inputString: props.text, isDirect: props.is_direct)
    case .delete_left:
      deleteLeft()
    case .delete_right:
      deleteRight()
    case .complete_prefix:
      completePrefix(candidateIndex: 1)  //DEBUG
    case .move_cursor:
      let props = try JSONDecoder().decode(MoveCursorProps.self, from: propsData)
      moveCursor(offset: props.offset)
    case .get_hiragana_with_cursor:
      let result = getHiraganaWithCursor()
      return String(data: try JSONEncoder().encode(result), encoding: .utf8) ?? ""
    case .get_composing_string:
      let props = try JSONDecoder().decode(GetComposingStringProps.self, from: propsData)
      let result = getComposingString(charType: props.char_type)
      let debugPrint =
        String(data: try JSONEncoder().encode(SimpleResult(result: result)), encoding: .utf8) ?? ""
      print(debugPrint)
      return debugPrint
    case .get_candidates:
      let result = getCandidates()
      return String(data: try JSONEncoder().encode(result), encoding: .utf8) ?? ""
    }
  } catch {
    NSLog("failed to parse JSON: \(error)")
    NSLog("Input: \(jsonString)")
    return "{}"
  }
  return "{}"
}

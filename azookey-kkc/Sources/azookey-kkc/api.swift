import Foundation
import KanaKanjiConverterModuleWithDefaultDictionary
import SwiftUtils

///
/// Config
///

@_silgen_name("kkc_get_config")
@MainActor public func getConfig(
  zenzaiEnabled: Bool, zenzaiInferLimit: Int,
  numberFullwidth: Int, symbolFullwidth: Int, periodStyleIndex: Int,
  commaStyleIndex: Int, spaceFullwidth: Int, tenCombining: Int, autoCommitModeNum: Int
) -> OpaquePointer? {
  let numberStyle: KkcConfig.Style = numberFullwidth == 1 ? .fullwidth : .halfwidth
  let symbolStyle: KkcConfig.Style = symbolFullwidth == 1 ? .fullwidth : .halfwidth
  let periodStyle: KkcConfig.TenStyle =
    periodStyleIndex == 1
    ? .halfwidthJapanese
    : periodStyleIndex == 2
      ? .fullwidthLatin : periodStyleIndex == 3 ? .halfwidthLatin : .fullwidthJapanese
  let commaStyle: KkcConfig.TenStyle =
    commaStyleIndex == 1
    ? .halfwidthJapanese
    : commaStyleIndex == 2
      ? .fullwidthLatin : commaStyleIndex == 3 ? .halfwidthLatin : .fullwidthJapanese
  let spaceStyle: KkcConfig.Style = spaceFullwidth == 1 ? .fullwidth : .halfwidth
  let diacriticStyle: KkcConfig.DiacriticStyle =
    tenCombining == 1
    ? .halfwidth
    : tenCombining == 2 ? .combining : .fullwidth

  let autoCommitMode: KkcConfig.AutoCommitMode
  switch autoCommitModeNum {
  case 1:
    autoCommitMode = .period
  case 2:
    autoCommitMode = .periodQuestionExclamation
  case 3:
    autoCommitMode = .periodCommaQuestionExclamation
  default:
    autoCommitMode = .none
  }

  let config = genDefaultConfig(
    zenzaiEnabled: zenzaiEnabled, zenzaiInferLimit: zenzaiInferLimit, numberStyle: numberStyle,
    symbolStyle: symbolStyle, periodStyle: periodStyle,
    commaStyle: commaStyle, spaceStyle: spaceStyle, diacriticStyle: diacriticStyle,
    autoCommitMode: autoCommitMode)
  let configPtr = Unmanaged.passRetained(config).toOpaque()
  return OpaquePointer(configPtr)
}

@_silgen_name("kkc_free_config")
public func freeConfig(ptr: OpaquePointer?) {
  guard let ptr = ptr else {
    return
  }
  Unmanaged<KkcConfig>.fromOpaque(UnsafeRawPointer(ptr)).release()
}

///
/// ComposingText
///

@_silgen_name("kkc_get_composing_text_instance")
@MainActor public func getComposingTextInstance() -> UnsafeMutablePointer<ComposingText>? {
  let composingTextPtr = UnsafeMutablePointer<ComposingText>.allocate(capacity: 1)
  let composingText = ComposingText()
  composingTextPtr.initialize(to: composingText)
  return composingTextPtr
}

@_silgen_name("kkc_free_composing_text_instance")
public func freeComposingTextInstance(ptr: UnsafeMutablePointer<ComposingText>?) {
  ptr?.deinitialize(count: 1)
  ptr?.deallocate()
}

@_silgen_name("kkc_input_text")
public func inputText(
  composingTextPtr: UnsafeMutablePointer<ComposingText>?, kkcConfigPtr: OpaquePointer?,
  stringPtr: UnsafePointer<Int8>?
) {
  guard let composingTextPtr = composingTextPtr else {
    return
  }
  guard let stringPtr = stringPtr else {
    return
  }

  guard var inputUnicode = String(cString: stringPtr).unicodeScalars.first else {
    return
  }

  let config: KkcConfig
  if let kkcConfigPtr = kkcConfigPtr {
    config = Unmanaged<KkcConfig>.fromOpaque(UnsafeRawPointer(kkcConfigPtr))
      .takeUnretainedValue()
  } else {
    config = genDefaultConfig(
      numberStyle: .fullwidth, symbolStyle: .fullwidth,
      periodStyle: .fullwidthJapanese, commaStyle: .fullwidthJapanese, spaceStyle: .fullwidth,
      diacriticStyle: .fullwidth,
      autoCommitMode: .none)
  }

  let inputStyle: InputStyle
  if inputUnicode.properties.isAlphabetic {
    inputStyle = .roman2kana
  } else {
    inputStyle = .direct
  }

  // convert katakana to hiragana
  if (0x30A0...0x30FF).contains(inputUnicode.value) {
    inputUnicode = UnicodeScalar(inputUnicode.value - 96)!
  }

  let inputCharacter: Character

  switch Character(inputUnicode) {
  case "1", "2", "3", "4", "5", "6", "7", "8", "9", "0":
    if config.numberStyle == .fullwidth {
      inputUnicode = UnicodeScalar(inputUnicode.value + 0xFEE0)!
      inputCharacter = Character(inputUnicode)
    } else {
      inputCharacter = Character(inputUnicode)
    }
  case "!", "\"", "#", "$", "%", "&", "'", "(", ")", "*", "+", "/", ":", ";", "<",
    "=", ">", "?", "@", "[", "\\", "]", "^", "_", "`", "{", "|", "}", "~":
    inputCharacter = symbolHalfwidthToFullwidth(
      character: Character(inputUnicode), reverse: config.symbolStyle == .halfwidth)
  case ".":
    switch config.periodStyle {
    case .fullwidthJapanese:
      inputCharacter = "。"
    case .halfwidthJapanese:
      inputCharacter = "｡"
    case .fullwidthLatin:
      inputCharacter = "．"
    case .halfwidthLatin:
      inputCharacter = "､"
    }
  case ",":
    switch config.commaStyle {
    case .fullwidthJapanese:
      inputCharacter = "、"
    case .halfwidthJapanese:
      inputCharacter = "､"
    case .fullwidthLatin:
      inputCharacter = "，"
    case .halfwidthLatin:
      inputCharacter = "､"
    }
  case "-":
    inputCharacter = "ー"
  default:
    inputCharacter = Character(inputUnicode)
  }

  var string = String(inputCharacter)

  switch string {
  case "゛":
    if let lastElem = composingTextPtr.pointee.input.last {
      let dakutened = CharacterUtils.dakuten(lastElem.character)
      if dakutened == lastElem.character {
        if lastElem.character != "゛" && lastElem.character != "゜" && lastElem.character != "゙"
          && lastElem.character != "゚" && lastElem.character != "ﾞ" && lastElem.character != "ﾟ"
        {
          composingTextPtr.pointee.deleteBackwardFromCursorPosition(count: 1)
          composingTextPtr.pointee.insertAtCursorPosition(
            String(lastElem.character), inputStyle: lastElem.inputStyle)
        }
        switch config.tenCombiningStyle {
        case .fullwidth:
          string = "゛"
        case .halfwidth:
          string = "ﾞ"
        case .combining:
          string = "゙"
        }
      } else {
        composingTextPtr.pointee.deleteBackwardFromCursorPosition(count: 1)
        string = String(dakutened)
      }
    }
  case "゜":
    if let lastElem = composingTextPtr.pointee.input.last {
      let handakutened = CharacterUtils.handakuten(lastElem.character)
      if handakutened == lastElem.character {
        if lastElem.character != "゛" && lastElem.character != "゜" && lastElem.character != "゙"
          && lastElem.character != "゚" && lastElem.character != "ﾞ" && lastElem.character != "ﾟ"
        {
          composingTextPtr.pointee.deleteBackwardFromCursorPosition(count: 1)
          composingTextPtr.pointee.insertAtCursorPosition(
            String(lastElem.character), inputStyle: lastElem.inputStyle)
        }
        switch config.tenCombiningStyle {
        case .fullwidth:
          string = "゜"
        case .halfwidth:
          string = "ﾟ"
        case .combining:
          string = "゚"
        }
      } else {
        composingTextPtr.pointee.deleteBackwardFromCursorPosition(count: 1)
        string = String(handakutened)
      }
    }
  default:
    break
  }

  composingTextPtr.pointee.insertAtCursorPosition(string, inputStyle: inputStyle)
}

@_silgen_name("kkc_delete_backward")
public func deleteBackward(composingTextPtr: UnsafeMutablePointer<ComposingText>?) {
  guard let composingTextPtr = composingTextPtr else {
    return
  }
  composingTextPtr.pointee.deleteBackwardFromCursorPosition(count: 1)
}

@_silgen_name("kkc_delete_forward")
public func deleteForward(composingTextPtr: UnsafeMutablePointer<ComposingText>?) {
  guard let composingTextPtr = composingTextPtr else {
    return
  }
  composingTextPtr.pointee.deleteForwardFromCursorPosition(count: 1)
}

@_silgen_name("kkc_complete_prefix")
public func completePrefix(
  composingTextPtr: UnsafeMutablePointer<ComposingText>?, correspondingCount: Int
) {
  guard let composingTextPtr = composingTextPtr else {
    return
  }
  composingTextPtr.pointee.prefixComplete(correspondingCount: correspondingCount)
}

@_silgen_name("kkc_move_cursor")
public func moveCursor(composingTextPtr: UnsafeMutablePointer<ComposingText>?, offset: Int)
  -> Int
{
  guard let composingTextPtr = composingTextPtr else {
    return 0
  }
  return composingTextPtr.pointee.moveCursorFromCursorPosition(count: offset)
}

///
/// ComposingText -> Characters
///

@_silgen_name("kkc_get_composing_hiragana")
public func getComposingHiragana(composingTextPtr: UnsafeMutablePointer<ComposingText>?)
  -> UnsafeMutablePointer<Int8>?
{
  guard let composingTextPtr = composingTextPtr else {
    return nil
  }
  return strdup(composingTextPtr.pointee.toHiragana())
}

@_silgen_name("kkc_get_composing_katakana_fullwidth")
public func getComposingKatakanaFullwidth(composingTextPtr: UnsafeMutablePointer<ComposingText>?)
  -> UnsafeMutablePointer<Int8>?
{
  guard let composingTextPtr = composingTextPtr else {
    return nil
  }
  return strdup(composingTextPtr.pointee.toKatakana(true))
}

@_silgen_name("kkc_get_composing_katakana_halfwidth")
public func getComposingKatakanaHalfwidth(composingTextPtr: UnsafeMutablePointer<ComposingText>?)
  -> UnsafeMutablePointer<Int8>?
{
  guard let composingTextPtr = composingTextPtr else {
    return nil
  }
  return strdup(composingTextPtr.pointee.toKatakana(false))
}

@_silgen_name("kkc_get_composing_alphabet_halfwidth")
public func getComposingAlphabetHalfwidth(
  composingTextPtr: UnsafeMutablePointer<ComposingText>?, currentPreeditPtr: UnsafePointer<Int8>?
)
  -> UnsafeMutablePointer<Int8>?
{
  guard let composingTextPtr = composingTextPtr else {
    return nil
  }
  guard let currentPreeditPtr = currentPreeditPtr else {
    return nil
  }
  let currentPreedit = String(cString: currentPreeditPtr)
  let alphabet = composingTextPtr.pointee.toAlphabet(false)
  return strdup(cycleAlphabetCase(alphabet, preedit: currentPreedit))
}

@_silgen_name("kkc_get_composing_alphabet_fullwidth")
public func getComposingAlphabetFullwidth(
  composingTextPtr: UnsafeMutablePointer<ComposingText>?, currentPreeditPtr: UnsafePointer<Int8>?
)
  -> UnsafeMutablePointer<Int8>?
{
  guard let composingTextPtr = composingTextPtr else {
    return nil
  }
  guard let currentPreeditPtr = currentPreeditPtr else {
    return nil
  }
  let currentPreedit = String(cString: currentPreeditPtr)
  let fullwidthAlphabet = composingTextPtr.pointee.toAlphabet(true)
  return strdup(cycleAlphabetCase(fullwidthAlphabet, preedit: currentPreedit))
}

@_silgen_name("kkc_free_text")
public func freeText(ptr: UnsafeMutablePointer<Int8>?) {
  free(ptr)
}

///
/// Candidates
///

@_silgen_name("kkc_get_candidates")
@MainActor public func getCandidates(
  composingTextPtr: UnsafeMutablePointer<ComposingText>?,
  kkcConfigPtr: OpaquePointer?,
  isPredictMode: Bool?, nBest: Int?
) -> UnsafeMutablePointer<UnsafeMutablePointer<UnsafeMutablePointer<Int8>?>?>? {
  guard let composingTextPtr = composingTextPtr else {
    return nil
  }
  if composingTextPtr.pointee.isEmpty {
    return nil
  }
  let composingText = composingTextPtr.pointee

  // get config
  let config: KkcConfig
  if let kkcConfigPtr = kkcConfigPtr {
    config = Unmanaged<KkcConfig>.fromOpaque(UnsafeRawPointer(kkcConfigPtr))
      .takeUnretainedValue()
  } else {
    config = genDefaultConfig(
      numberStyle: .fullwidth, symbolStyle: .fullwidth,
      periodStyle: .fullwidthJapanese, commaStyle: .fullwidthJapanese, spaceStyle: .fullwidth,
      diacriticStyle: .fullwidth,
      autoCommitMode: .none)
  }

  // set options
  var options = config.convertOptions
  if nBest != nil {
    options.N_best = nBest!
  }
  if isPredictMode != nil && isPredictMode! {
    options.requireJapanesePrediction = true
    options.requireEnglishPrediction = true
  }

  let result = createCandidateStruct(composingText: composingText, options: options)

  let resultPtr = UnsafeMutablePointer<UnsafeMutablePointer<UnsafeMutablePointer<Int8>?>?>
    .allocate(
      capacity: result.count + 1)
  resultPtr.initialize(from: result, count: result.count)
  resultPtr[result.count] = nil

  return resultPtr
}

@_silgen_name("kkc_free_candidates")
public func freeCandidates(
  ptr: UnsafeMutablePointer<UnsafeMutablePointer<UnsafeMutablePointer<Int8>?>?>?
) {
  guard let ptr = ptr else {
    return
  }
  var i = 0
  while ptr[i] != nil {
    var j = 0
    while ptr[i]![j] != nil {
      free(ptr[i]![j])
      j += 1
    }
    ptr[i]?.deallocate()
    i += 1
  }
  ptr.deallocate()
}

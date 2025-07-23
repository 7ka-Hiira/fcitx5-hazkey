import Foundation
import KanaKanjiConverterModule
import SwiftUtils

/// Config

// TODO: 数字に変換せず直接スタイルをわたす
@MainActor public func setConfig(
  zenzaiEnabled: Bool, zenzaiInferLimit: Int,
  numberFullwidth: Int, symbolFullwidth: Int, periodStyleIndex: Int,
  commaStyleIndex: Int, spaceFullwidth: Int, tenCombining: Int,
  profileText: String
) {
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

  let config = genDefaultConfig(
    zenzaiEnabled: zenzaiEnabled, zenzaiInferLimit: zenzaiInferLimit, numberStyle: numberStyle,
    symbolStyle: symbolStyle, periodStyle: periodStyle,
    commaStyle: commaStyle, spaceStyle: spaceStyle, diacriticStyle: diacriticStyle,
    profileText: profileText)

  kkcConfig = config

  // preload model to avoid delay on first input
  if zenzaiEnabled {
    // var dummyComposingText = ComposingText()
    // dummyComposingText.insertAtCursorPosition("a", inputStyle: .direct)
    // let _ = config.converter.requestCandidates(
    //   dummyComposingText, options: config.convertOptions)
  }

  return
}

@MainActor public func setLeftContext(
  surroundingText: String, anchorIndex: Int
) {
  guard let config = kkcConfig, config.convertOptions.zenzaiMode != .off else {
    return
  }

  let leftContext = String(surroundingText.prefix(anchorIndex))

  config.convertOptions.zenzaiMode = .on(
    weight: config.zenzaiWeight,
    personalizationMode: nil,  // TODO: handle this correctly
    versionDependentMode: .v3(.init(profile: config.profileText, leftSideContext: leftContext))
  )
}

/// ComposingText

@MainActor public func createComposingTextInstanse() {
  composingText = ComposingText()
}

@MainActor public func inputText(
  inputString: String, isDirect: Bool
) {
  guard var inputUnicode = inputString.unicodeScalars.first else {
    return
  }

  guard let config = kkcConfig else {
    return
  }

  guard var composingText = composingText else {
    return
  }

  if !isDirect {
    // convert katakana to hiragana
    if (0x30A0...0x30F3).contains(inputUnicode.value) {
      inputUnicode = UnicodeScalar(inputUnicode.value - 96)!
    }

    var inputCharacter: Character

    switch Character(inputUnicode) {
    case "1", "2", "3", "4", "5", "6", "7", "8", "9", "0":
      if config.numberStyle == .fullwidth {
        inputUnicode = UnicodeScalar(inputUnicode.value + 0xFEE0)!
        inputCharacter = Character(inputUnicode)
      } else {
        inputCharacter = Character(inputUnicode)
      }
    case "!", "\"", "#", "$", "%", "&", "'", "(", ")", "*", "+", "/", ":", ";", "<",
      "=", ">", "?", "@", "[", "\\", "]", "^", "_", "`", "{", "|", "}", "~", "¥":
      if isDirect {
        inputCharacter = Character(inputUnicode)
      } else {
        inputCharacter = symbolHalfwidthToFullwidth(
          character: Character(inputUnicode), reverse: config.symbolStyle == .halfwidth)
      }
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
      if let lastElem = composingText.input.last {
        let dakutened = CharacterUtils.dakuten(lastElem.character)
        if dakutened == lastElem.character {
          if lastElem.character != "゛" && lastElem.character != "゜"
            && lastElem.character != "゙"
            && lastElem.character != "゚" && lastElem.character != "ﾞ"
            && lastElem.character != "ﾟ"
          {
            composingText.deleteBackwardFromCursorPosition(count: 1)
            composingText.insertAtCursorPosition(
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
          composingText.deleteBackwardFromCursorPosition(count: 1)
          string = String(dakutened)
        }
      }
    case "゜":
      if let lastElem = composingText.input.last {
        let handakutened = CharacterUtils.handakuten(lastElem.character)
        if handakutened == lastElem.character {
          if lastElem.character != "゛" && lastElem.character != "゜"
            && lastElem.character != "゙"
            && lastElem.character != "゚" && lastElem.character != "ﾞ"
            && lastElem.character != "ﾟ"
          {
            composingText.deleteBackwardFromCursorPosition(count: 1)
            composingText.insertAtCursorPosition(
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
          composingText.deleteBackwardFromCursorPosition(count: 1)
          string = String(handakutened)
        }
      }
    default:
      break
    }
    composingText.insertAtCursorPosition(string, inputStyle: .roman2kana)
  } else {
    composingText.insertAtCursorPosition(String(inputUnicode), inputStyle: .direct)
  }
}

@MainActor public func deleteBackward() {
  guard var composingText = composingText else {
    return
  }
  composingText.deleteBackwardFromCursorPosition(count: 1)
}

@MainActor public func deleteForward() {
  guard var composingText = composingText else {
    return
  }
  composingText.deleteForwardFromCursorPosition(count: 1)
}

@MainActor public func completePrefix(composingCount: ComposingCount) {
  guard var composingText = composingText else {
    return
  }
  composingText.prefixComplete(composingCount: composingCount)
}

@MainActor public func moveCursor(offset: Int) -> Int {
  guard var composingText = composingText else {
    return 0
  }
  return composingText.moveCursorFromCursorPosition(count: offset)
}

/// ComposingText -> Characters

// TODO: ひとつにまとめてswitch charTypeにする
@MainActor public func getComposingHiragana() -> String {
  guard let composingText = composingText else {
    return ""
  }
  return composingText.toHiragana()
}

@MainActor public func getComposingHiraganaWithCursor() -> String {
  guard let composingText = composingText else {
    return ""
  }
  var hiragana = composingText.toHiragana()
  let cursorPos = composingText.convertTargetCursorPosition
  hiragana.insert("|", at: hiragana.index(hiragana.startIndex, offsetBy: cursorPos))
  return hiragana
}

@MainActor public func getComposingKatakanaFullwidth() -> String {
  guard let composingText = composingText else {
    return ""
  }
  return composingText.toKatakana(true)
}

@MainActor public func getComposingKatakanaHalfwidth() -> String {
  guard let composingText = composingText else {
    return ""
  }
  return composingText.toKatakana(false)
}

@MainActor public func getComposingAlphabetHalfwidth() -> String {
  guard let composingText = composingText else {
    return ""
  }
  let alphabet = composingText.toAlphabet(false)
  return cycleAlphabetCase(alphabet, preedit: currentPreedit)
}

@MainActor public func getComposingAlphabetFullwidth() -> String {
  guard let composingText = composingText else {
    return nil
  }
  let fullwidthAlphabet = composingText.toAlphabet(true)
  return cycleAlphabetCase(fullwidthAlphabet, preedit: currentPreedit)
}

/// Candidates

// TODO: return error message
@MainActor
public func getCandidates(  // isPredictMode: Bool?, nBest: Int?
  ) -> Data?
{  // JSON response
  guard var composingText = composingText else {
    return nil
  }

  guard var config = kkcConfig else {
    return nil
  }

  // set options
  var options = config.convertOptions
  // options.N_best = nBest!
  // options.requireJapanesePrediction = true
  // options.requireEnglishPrediction = true

  let result = createCandidateStruct(
    composingText: composingText, options: options, converter: config.converter)

  return try? JSONEncoder().encode(result)
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

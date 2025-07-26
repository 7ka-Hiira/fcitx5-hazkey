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
  composingText = ComposingTextBox(ComposingText())
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

  guard let composingText = composingText else {
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
      if let lastElem = composingText.value.input.last {
        let dakutened = CharacterUtils.dakuten(lastElem.character)
        if dakutened == lastElem.character {
          if lastElem.character != "゛" && lastElem.character != "゜"
            && lastElem.character != "゙"
            && lastElem.character != "゚" && lastElem.character != "ﾞ"
            && lastElem.character != "ﾟ"
          {
            composingText.value.deleteBackwardFromCursorPosition(count: 1)
            composingText.value.insertAtCursorPosition(
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
          composingText.value.deleteBackwardFromCursorPosition(count: 1)
          string = String(dakutened)
        }
      }
    case "゜":
      if let lastElem = composingText.value.input.last {
        let handakutened = CharacterUtils.handakuten(lastElem.character)
        if handakutened == lastElem.character {
          if lastElem.character != "゛" && lastElem.character != "゜"
            && lastElem.character != "゙"
            && lastElem.character != "゚" && lastElem.character != "ﾞ"
            && lastElem.character != "ﾟ"
          {
            composingText.value.deleteBackwardFromCursorPosition(count: 1)
            composingText.value.insertAtCursorPosition(
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
          composingText.value.deleteBackwardFromCursorPosition(count: 1)
          string = String(handakutened)
        }
      }
    default:
      break
    }
    composingText.value.insertAtCursorPosition(string, inputStyle: .roman2kana)
  } else {
    composingText.value.insertAtCursorPosition(String(inputUnicode), inputStyle: .direct)
  }
}

@MainActor public func deleteLeft() {
  guard let composingText = composingText else {
    return
  }
  composingText.value.deleteBackwardFromCursorPosition(count: 1)
}

@MainActor public func deleteRight() {
  guard let composingText = composingText else {
    return
  }
  composingText.value.deleteForwardFromCursorPosition(count: 1)
}

@MainActor public func completePrefix(candidateIndex: Int) {
  guard let composingText = composingText else {
    return
  }
  guard let currentCandidateList = currentCandidateList else {
    return
  }
  let completedCandidate = currentCandidateList[candidateIndex]
  composingText.value.prefixComplete(composingCount: completedCandidate.composingCount)
}

@MainActor public func moveCursor(offset: Int) {
  guard let composingText = composingText else {
    return
  }
  let _ = composingText.value.moveCursorFromCursorPosition(count: offset)
}

/// ComposingText -> Characters

@MainActor public func getHiraganaWithCursor() -> String {
  guard let composingText = composingText else {
    return ""
  }
  var hiragana = composingText.value.toHiragana()
  let cursorPos = composingText.value.convertTargetCursorPosition
  hiragana.insert("|", at: hiragana.index(hiragana.startIndex, offsetBy: cursorPos))
  return hiragana
}

public enum CharType: String, Decodable {
  case hiragana
  case katakana_fullwidth
  case katakana_halfwidth
  case alphabet_fullwidth
  case alphabet_halfwidth
}

@MainActor public func getComposingString(charType: CharType) -> String {
  guard let composingText = composingText else {
    return ""
  }
  switch charType {
  case .hiragana:
    return composingText.value.toHiragana()
  case .katakana_fullwidth:
    return composingText.value.toKatakana(true)
  case .katakana_halfwidth:
    return composingText.value.toKatakana(false)
  case .alphabet_fullwidth:
    return cycleAlphabetCase(composingText.value.toAlphabet(true), preedit: currentPreedit)
  case .alphabet_halfwidth:
    return cycleAlphabetCase(composingText.value.toAlphabet(false), preedit: currentPreedit)
  }
}

/// Candidates

// TODO: return error message
@MainActor
public func getCandidates(isPredictMode: Bool = false, nBest: Int = 9) -> Data? {  // JSON response
  guard let composingText = composingText else {
    return nil
  }

  guard let config = kkcConfig else {
    return nil
  }

  var options = config.convertOptions
  options.N_best = nBest
  options.requireJapanesePrediction = isPredictMode
  options.requireEnglishPrediction = isPredictMode

  let converted = config.converter.requestCandidates(composingText.value, options: options)

  let hiraganaPreedit = composingText.value.toHiragana()

  var result: [FcitxCandidate] = []
  for candidate in converted.mainResults {


    let candidateLen = candidate.rubyCount
    let subHiragana: String
    if let idx = hiraganaPreedit.index(hiraganaPreedit.startIndex, offsetBy: candidateLen, limitedBy: hiraganaPreedit.endIndex) {
        subHiragana = String(hiraganaPreedit[idx...])
    } else {
        subHiragana = ""
    }

    let fcitxCandidate = FcitxCandidate(t: candidate.text, h: subHiragana)
    result.append(fcitxCandidate)
  }

  return try? JSONEncoder().encode(result)
}

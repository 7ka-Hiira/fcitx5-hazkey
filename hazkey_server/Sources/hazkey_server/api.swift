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
  // composingText = ComposingTextBox()
}

@MainActor func inputText(
  inputString: String, isDirect: Bool
) -> Hazkey_SimpleResult {
  guard var inputUnicode = inputString.unicodeScalars.first else {
    return Hazkey_SimpleResult.with {
      $0.status = .failed
      $0.result = "failed to get first unicode character"
    }
  }

  guard let config = kkcConfig else {
    return Hazkey_SimpleResult.with {
      $0.status = .failed
      $0.result = "config not found"
    }
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
  return Hazkey_SimpleResult.with { $0.status = .success }
}

@MainActor public func deleteLeft() {
  composingText.value.deleteBackwardFromCursorPosition(count: 1)
}

@MainActor public func deleteRight() {
  composingText.value.deleteForwardFromCursorPosition(count: 1)
}

@MainActor public func completePrefix(candidateIndex: Int) {
  if let completedCandidate = currentCandidateList?[candidateIndex] {
  composingText.value.prefixComplete(composingCount: completedCandidate.composingCount)
  } else {
  return //TODO: return error
  }
}

@MainActor public func moveCursor(offset: Int) {
  let _ = composingText.value.moveCursorFromCursorPosition(count: offset)
}

/// ComposingText -> Characters

@MainActor public func getHiraganaWithCursor() -> String {
  var hiragana = composingText.value.toHiragana()
  let cursorPos = composingText.value.convertTargetCursorPosition
  hiragana.insert("|", at: hiragana.index(hiragana.startIndex, offsetBy: cursorPos))
  return hiragana
}

@MainActor func getComposingString(charType: Hazkey_CharType) -> Hazkey_SimpleResult {
  let result: String
  switch charType {
  case .hiragana:
    result = composingText.value.toHiragana()
  case .katakanaFull:
    result = composingText.value.toKatakana(true)
  case .katakanaHalf:
    result = composingText.value.toKatakana(false)
  case .alphabetFull:
    result = cycleAlphabetCase(
      composingText.value.toAlphabet(true), preedit: currentPreedit)
  case .alphabetHalf:
    result = cycleAlphabetCase(
      composingText.value.toAlphabet(false), preedit: currentPreedit)
  case .UNRECOGNIZED(_):
    return Hazkey_SimpleResult.with {
      $0.status = .failed
      $0.result = "unrecognized charType"
    }
  }
  return Hazkey_SimpleResult.with {
    $0.status = .success
    $0.result = result
  }
}

/// Candidates

// TODO: return error message
@MainActor
func getCandidates(isPredictMode: Bool = false, nBest: Int = 9) -> Hazkey_CandidatesResult {

  guard let config = kkcConfig else {
    return Hazkey_CandidatesResult.init()
  }

  var options = config.convertOptions
  options.N_best = nBest
  options.requireJapanesePrediction = isPredictMode
  options.requireEnglishPrediction = isPredictMode

  let converted = config.converter.requestCandidates(composingText.value, options: options)

  let hiraganaPreedit = composingText.value.toHiragana()

  var result = Hazkey_CandidatesResult()
  result.candidates = converted.mainResults.map { c in
    var candidate = Hazkey_Candidate()
    candidate.text = c.text
    if let idx = hiraganaPreedit.index(
      hiraganaPreedit.startIndex, offsetBy: c.rubyCount, limitedBy: hiraganaPreedit.endIndex)
    {
      candidate.subHiragana = String(hiraganaPreedit[idx...])
    } else {
      candidate.subHiragana = ""
    }

    return candidate
  }

  return result
}

import Foundation
import KanaKanjiConverterModule
import SwiftUtils

@MainActor func setContext(
  surroundingText: String, anchorIndex: Int
) -> Hazkey_ResponseEnvelope {
  let leftContext = String(surroundingText.prefix(anchorIndex))
  baseConvertRequestOptions.zenzaiMode = genZenzaiMode(leftContext: leftContext)

  return Hazkey_ResponseEnvelope.with {
    $0.status = .success
  }
}

/// ComposingText

@MainActor func createComposingTextInstanse() -> Hazkey_ResponseEnvelope {
  composingText = ComposingTextBox()
  currentCandidateList = nil
  return Hazkey_ResponseEnvelope.with {
    $0.status = .success
  }
}

@MainActor func inputChar(
  inputString: String, isDirect: Bool
) -> Hazkey_ResponseEnvelope {
  guard var inputUnicode = inputString.unicodeScalars.first else {
    return Hazkey_ResponseEnvelope.with {
      $0.status = .failed
      $0.errorMessage = "failed to get first unicode character"
    }
  }

  if !isDirect {
    // convert katakana to hiragana
    if (0x30A0...0x30F3).contains(inputUnicode.value) {
      inputUnicode = UnicodeScalar(inputUnicode.value - 96)!
    }
    composingText.value.insertAtCursorPosition(String(inputUnicode), inputStyle: .roman2kana)
  } else {
    composingText.value.insertAtCursorPosition(String(inputUnicode), inputStyle: .direct)
  }
  return Hazkey_ResponseEnvelope.with { $0.status = .success }
}

@MainActor func deleteLeft() -> Hazkey_ResponseEnvelope {
  composingText.value.deleteBackwardFromCursorPosition(count: 1)
  return Hazkey_ResponseEnvelope.with {
    $0.status = .success
  }
}

@MainActor func deleteRight() -> Hazkey_ResponseEnvelope {
  composingText.value.deleteForwardFromCursorPosition(count: 1)
  return Hazkey_ResponseEnvelope.with {
    $0.status = .success
  }
}

@MainActor func completePrefix(candidateIndex: Int) -> Hazkey_ResponseEnvelope {
  if let completedCandidate = currentCandidateList?[candidateIndex] {
    composingText.value.prefixComplete(composingCount: completedCandidate.composingCount)
  } else {
    return Hazkey_ResponseEnvelope.with {
      $0.status = .failed
      $0.errorMessage = "Candidate index \(candidateIndex) not found."
    }
  }
  return Hazkey_ResponseEnvelope.with {
    $0.status = .success
  }
}

@MainActor func moveCursor(offset: Int) -> Hazkey_ResponseEnvelope {
  let _ = composingText.value.moveCursorFromCursorPosition(count: offset)
  return Hazkey_ResponseEnvelope.with {
    $0.status = .success
  }
}

/// ComposingText -> Characters

@MainActor func getHiraganaWithCursor() -> Hazkey_ResponseEnvelope {
  var hiragana = composingText.value.toHiragana()
  let cursorPos = composingText.value.convertTargetCursorPosition
  if !hiragana.isEmpty {
    hiragana.insert("|", at: hiragana.index(hiragana.startIndex, offsetBy: cursorPos))
  }
  return Hazkey_ResponseEnvelope.with {
    $0.status = .success
    $0.text = hiragana
  }
}

@MainActor func getComposingString(
  charType: Hazkey_Commands_GetComposingString.CharType,
  currentPreedit: String
) -> Hazkey_ResponseEnvelope {
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
    return Hazkey_ResponseEnvelope.with {
      $0.status = .failed
      $0.errorMessage = "unrecognized charType: \(charType.rawValue)"
    }
  }
  return Hazkey_ResponseEnvelope.with {
    $0.status = .success
    $0.text = result
  }
}

/// Candidates

// TODO: return error message
@MainActor
func getCandidates(isPredictMode: Bool = false, nBest: Int = 9) -> Hazkey_ResponseEnvelope {
  var options = baseConvertRequestOptions
  options.N_best = nBest
  options.requireJapanesePrediction = isPredictMode
  options.requireEnglishPrediction = isPredictMode

  let converted = converter.requestCandidates(composingText.value, options: options)

  currentCandidateList = converted.mainResults

  let hiraganaPreedit = composingText.value.toHiragana()

  var candidatesResult = Hazkey_Commands_CandidatesResult()
  candidatesResult.candidates = converted.mainResults.map { c in
    var candidate = Hazkey_Commands_CandidatesResult.Candidate()

    candidate.text = c.text

    if let idx = hiraganaPreedit.index(
      hiraganaPreedit.startIndex, offsetBy: c.rubyCount, limitedBy: hiraganaPreedit.endIndex)
    {
      candidate.subHiragana = String(hiraganaPreedit[idx...])
    } else {
      candidate.subHiragana = ""
    }

    if candidatesResult.liveText.isEmpty && c.rubyCount == hiraganaPreedit.count {
      candidatesResult.liveText = c.text
    }

    return candidate
  }

  return Hazkey_ResponseEnvelope.with {
    $0.status = .success
    $0.candidates = candidatesResult
  }
}

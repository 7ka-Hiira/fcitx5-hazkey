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
    guard let inputChar = inputString.first else {
        return Hazkey_ResponseEnvelope.with {
            $0.status = .failed
            $0.errorMessage = "failed to get first unicode character"
        }
    }
    if !isDirect {
        let piece: InputPiece
        if let (intentionChar, overrideInputChar) = keymap[inputChar] {
            piece = .key(
                intention: intentionChar, input: overrideInputChar ?? inputChar, modifiers: [])
        } else {
            piece = .character(inputChar)
        }

        composingText.value.insertAtCursorPosition([
            ComposingText.InputElement.init(
                piece: piece,
                inputStyle: .mapped(id: .tableName(currentTableName)))
        ])
    } else {
        composingText.value.insertAtCursorPosition(String(inputChar), inputStyle: .direct)
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
    _ = composingText.value.moveCursorFromCursorPosition(count: offset)
    return Hazkey_ResponseEnvelope.with {
        $0.status = .success
    }
}

/// ComposingText -> Characters

@MainActor func getHiraganaWithCursor() -> Hazkey_ResponseEnvelope {
    func safeSubstring(_ text: String, start: Int, end: Int) -> String {
        guard start >= 0, end >= 0, start < text.count, end <= text.count, start < end else {
            return ""
        }

        let startIndex = text.index(text.startIndex, offsetBy: start)
        let endIndex = text.index(text.startIndex, offsetBy: end)

        return String(text[startIndex..<endIndex])
    }

    let hiragana = composingText.value.toHiragana()
    let cursorPos = composingText.value.convertTargetCursorPosition

    if hiragana.count == cursorPos {
        return Hazkey_ResponseEnvelope.with {
            $0.status = .success
            $0.textWithCursor = Hazkey_Commands_TextWithCursor.with {
                $0.beforeCursosr = ""
                $0.onCursor = ""
                $0.afterCursor = ""
            }
        }
    }

    return Hazkey_ResponseEnvelope.with {
        $0.status = .success
        $0.textWithCursor = Hazkey_Commands_TextWithCursor.with {
            $0.beforeCursosr = safeSubstring(hiragana, start: 0, end: cursorPos)
            $0.onCursor = safeSubstring(hiragana, start: cursorPos, end: cursorPos + 1)
            $0.afterCursor = safeSubstring(hiragana, start: cursorPos + 1, end: hiragana.count)
        }
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
    case .UNRECOGNIZED:
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
func getCandidates(is_suggest: Bool) -> Hazkey_ResponseEnvelope {
    var options = baseConvertRequestOptions
    options.N_best = {
        if is_suggest
            && currentProfile.suggestionListMode
                == Hazkey_Config_Profile.SuggestionListMode.suggestionListDisabled
        {
            // for auto conversion
            return 1
        } else if is_suggest {
            return Int(currentProfile.numSuggestions)
        } else {
            return Int(currentProfile.numCandidatesPerPage)
        }
    }()

    options.requireJapanesePrediction =
        is_suggest
        && currentProfile.suggestionListMode
            == Hazkey_Config_Profile.SuggestionListMode.suggestionListShowPredictiveResults
    options.requireEnglishPrediction = options.requireJapanesePrediction

    let converted = converter.requestCandidates(composingText.value, options: options)

    currentCandidateList = converted.mainResults

    let hiraganaPreedit = composingText.value.toHiragana()

    var candidatesResult = Hazkey_Commands_CandidatesResult()
    candidatesResult.liveTextIndex = -1
    candidatesResult.candidates = converted.mainResults.enumerated().map { index, c in
        var candidate = Hazkey_Commands_CandidatesResult.Candidate()
        candidate.text = c.text

        let endIndex = min(c.rubyCount, hiraganaPreedit.count)
        candidate.subHiragana = String(hiraganaPreedit.dropFirst(endIndex))

        // Set liveText if conditions are met
        if candidatesResult.liveText.isEmpty && c.rubyCount == hiraganaPreedit.count {
            candidatesResult.liveText = c.text
            candidatesResult.liveTextIndex = Int32(index)
        }

        return candidate
    }

    // Do not automatically convert if there is only one character
    if currentProfile.autoConvertMode
        == Hazkey_Config_Profile.AutoConvertMode.autoConvertForMultipleChars
        && hiraganaPreedit.count == 1
    {
        candidatesResult.liveText = ""
        candidatesResult.liveTextIndex = -1
    } else if currentProfile.autoConvertMode
        == Hazkey_Config_Profile.AutoConvertMode.autoConvertDisabled
    {
        candidatesResult.liveText = ""
        candidatesResult.liveTextIndex = -1
    }

    candidatesResult.pageSize = {
        if is_suggest
            && currentProfile.suggestionListMode
                == Hazkey_Config_Profile.SuggestionListMode.suggestionListDisabled
        {
            return 0
        } else if is_suggest {
            return currentProfile.numSuggestions
        } else {
            return currentProfile.numCandidatesPerPage
        }
    }()

    return Hazkey_ResponseEnvelope.with {
        $0.status = .success
        $0.candidates = candidatesResult
    }
}

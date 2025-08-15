import Foundation
import KanaKanjiConverterModule

extension ComposingText {
  func toHiragana() -> String {
    return self.convertTarget
  }

  func toKatakana(_ fullwidth: Bool) -> String {
    let hiragana = self.toHiragana()
    let katakanaFullwidth =
      hiragana.applyingTransform(.hiraganaToKatakana, reverse: false) ?? hiragana
    if fullwidth {
      return katakanaFullwidth
    } else {
      return katakanaFullwidth.applyingTransform(.fullwidthToHalfwidth, reverse: false)
        ?? katakanaFullwidth
    }
  }

  func toAlphabet(_ fullwidth: Bool) -> String {
    let romaji = self.input.compactMap {
      if case let .character(character) = $0.piece {
        return character
      }
      return nil
    }
    return String(romaji).applyingTransform(.fullwidthToHalfwidth, reverse: fullwidth) ?? ""
  }
}

func cycleAlphabetCase(_ alphabet: String, preedit: String) -> String {
  if preedit == alphabet.lowercased() {
    return alphabet.uppercased()
  } else if preedit == alphabet.uppercased() && alphabet.count > 1 {
    return alphabet.capitalized
  } else if alphabet != alphabet.uppercased()
    && alphabet != alphabet.lowercased()
    && alphabet != alphabet.capitalized
    && alphabet != preedit
  {
    return alphabet
  } else {
    return alphabet.lowercased()
  }
}

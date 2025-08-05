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
    var beforeCharacter: Character?
    // reverse to process "っ" correctly
    // reverse back to original order
    let romaji = self.input.reversed().map {
      // convert symbol first because applyingTransform doesn't work for them
      var character = symbolJaToEn(
        character: symbolHalfwidthToFullwidth(
          character: symbolJaToEn(character: $0.character, reverse: false), reverse: fullwidth),
        reverse: false)
      switch character {
      case "ん":
        character = "n"
      case "っ":
        if let beforeCharacter = beforeCharacter {
          character = beforeCharacter
        }
      default:
        break
      }
      beforeCharacter = character
      return String(character)
    }.reversed().joined()
    return romaji.applyingTransform(.fullwidthToHalfwidth, reverse: fullwidth) ?? ""
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

func symbolJaToEn(character: Character, reverse: Bool) -> Character {
  let h2z: [Character: Character] = [
    "。": "．",
    "、": "，",
    "・": "／",
    "「": "［",
    "」": "］",
    "￥": "＼",
    "｡": ".",
    "､": ",",
    "･": "/",
    "｢": "[",
    "｣": "]",
    "¥": "\\",
  ]
  if reverse {
    let z2h = Dictionary(uniqueKeysWithValues: h2z.map { ($1, $0) })
    return z2h[character] ?? character
  } else {
    return h2z[character] ?? character
  }
}

func symbolHalfwidthToFullwidth(character: Character, reverse: Bool) -> Character {
  let h2z: [Character: Character] = [
    "!": "！",
    "\"": "”",
    "#": "＃",
    "$": "＄",
    "%": "％",
    "&": "＆",
    "'": "’",
    "(": "（",
    ")": "）",
    "=": "＝",
    "~": "〜",
    "|": "｜",
    "`": "｀",
    "{": "『",
    "+": "＋",
    "*": "＊",
    "}": "』",
    "<": "＜",
    ">": "＞",
    "?": "？",
    "_": "＿",
    "-": "ー",
    "^": "＾",
    "\\": "＼",
    "¥": "￥",
    "@": "＠",
    "[": "「",
    ";": "；",
    ":": "：",
    "]": "」",
    "/": "・",
    ",": "、",
    ".": "。",
  ]
  if reverse {
    let z2h = Dictionary(uniqueKeysWithValues: h2z.map { ($1, $0) })
    return z2h[character] ?? character
  } else {
    return h2z[character] ?? character
  }
}

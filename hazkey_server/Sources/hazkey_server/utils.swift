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

public class KkcConfig {
  enum Style {
    case halfwidth
    case fullwidth
  }

  enum TenStyle {
    case fullwidthJapanese
    case halfwidthJapanese
    case fullwidthLatin
    case halfwidthLatin
  }

  enum DiacriticStyle {
    case fullwidth
    case halfwidth
    case combining
  }

  var convertOptions: ConvertRequestOptions

  let numberStyle: Style
  let symbolStyle: Style
  let periodStyle: TenStyle
  let commaStyle: TenStyle
  let spaceStyle: Style

  let tenCombiningStyle: DiacriticStyle

  let zenzaiWeight: URL
  let profileText: String?

  let converter: KanaKanjiConverter

  @MainActor init(
    convertOptions: ConvertRequestOptions, numberStyle: Style, symbolStyle: Style,
    periodStyle: TenStyle, commaStyle: TenStyle, spaceStyle: Style, diacriticStyle: DiacriticStyle,
    zenzaiWeight: URL, profileText: String?
  ) {
    self.convertOptions = convertOptions
    self.numberStyle = numberStyle
    self.symbolStyle = symbolStyle
    self.periodStyle = periodStyle
    self.commaStyle = commaStyle
    self.spaceStyle = spaceStyle
    self.tenCombiningStyle = diacriticStyle
    self.zenzaiWeight = zenzaiWeight
    self.profileText = profileText
    self.converter = KanaKanjiConverter()
  }
}

@MainActor func genDefaultConfig(
  zenzaiEnabled: Bool = false, zenzaiInferLimit: Int = 1, numberStyle: KkcConfig.Style,
  symbolStyle: KkcConfig.Style,
  periodStyle: KkcConfig.TenStyle, commaStyle: KkcConfig.TenStyle, spaceStyle: KkcConfig.Style,
  diacriticStyle: KkcConfig.DiacriticStyle, profileText: String?
) -> KkcConfig {
  var userDataDir: URL {
    FileManager.default.urls(for: .applicationSupportDirectory, in: .userDomainMask).first!
      .appendingPathComponent("hazkey", isDirectory: true)
  }

  var systemResourceDir: URL {
    URL(fileURLWithPath: systemResourcePath, isDirectory: true)
  }

  do {
    try FileManager.default.createDirectory(at: userDataDir, withIntermediateDirectories: true)
  } catch {
    print("Error creating directory: \(error)")
  }

  let options = ConvertRequestOptions(
    N_best: 9,
    needTypoCorrection: nil,
    requireJapanesePrediction: false,
    requireEnglishPrediction: false,
    keyboardLanguage: .ja_JP,
    englishCandidateInRoman2KanaInput: false,
    fullWidthRomanCandidate: true,
    halfWidthKanaCandidate: true,
    learningType: .nothing,
    maxMemoryCount: 65536,
    shouldResetMemory: true,
    dictionaryResourceURL: systemResourceDir.appendingPathComponent(
      "Dictionary", isDirectory: true),
    memoryDirectoryURL: userDataDir,
    sharedContainerURL: userDataDir,
    textReplacer: .init(emojiDataProvider: {
      systemResourceDir.appendingPathComponent(
        "emoji_all_E16.0.txt", isDirectory: false)
    }),
    specialCandidateProviders: nil,
    zenzaiMode: zenzaiEnabled
      ? .on(
        weight: systemResourceDir.appendingPathComponent("zenzai.gguf", isDirectory: false),
        inferenceLimit: zenzaiInferLimit, personalizationMode: nil,
        versionDependentMode: .v3(.init(profile: profileText))) : .off,
    metadata: ConvertRequestOptions.Metadata(
      versionString: "fcitx5-hazkey 0.1.0"
    ),
  )
  return KkcConfig(
    convertOptions: options, numberStyle: numberStyle, symbolStyle: symbolStyle,
    periodStyle: periodStyle, commaStyle: commaStyle, spaceStyle: spaceStyle,
    diacriticStyle: diacriticStyle,
    zenzaiWeight: systemResourceDir.appendingPathComponent("zenzai.gguf", isDirectory: false),
    profileText: profileText
  )
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

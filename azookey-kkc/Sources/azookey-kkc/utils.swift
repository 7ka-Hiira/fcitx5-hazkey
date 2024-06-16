import Foundation
import KanaKanjiConverterModuleWithDefaultDictionary

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
      var character = symbolHalfwidthToFullwidth(character: $0.character, reverse: fullwidth)
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

  enum AutoCommitMode {
    case none
    case period
    case periodQuestionExclamation
    case periodCommaQuestionExclamation
  }

  let convertOptions: ConvertRequestOptions

  let numberStyle: Style
  let symbolStyle: Style
  let periodStyle: TenStyle
  let commaStyle: TenStyle
  let spaceStyle: Style

  let tenCombiningStyle: DiacriticStyle

  let autoCommitMode: AutoCommitMode

  init(
    convertOptions: ConvertRequestOptions, numberStyle: Style, symbolStyle: Style,
    periodStyle: TenStyle, commaStyle: TenStyle, spaceStyle: Style, diacriticStyle: DiacriticStyle,
    autoCommitMode: AutoCommitMode
  ) {
    self.convertOptions = convertOptions
    self.numberStyle = numberStyle
    self.symbolStyle = symbolStyle
    self.periodStyle = periodStyle
    self.commaStyle = commaStyle
    self.spaceStyle = spaceStyle
    self.tenCombiningStyle = diacriticStyle
    self.autoCommitMode = autoCommitMode
  }
}

func genDefaultConfig(
  zenzaiEnabled: Bool = false, numberStyle: KkcConfig.Style, symbolStyle: KkcConfig.Style,
  periodStyle: KkcConfig.TenStyle, commaStyle: KkcConfig.TenStyle, spaceStyle: KkcConfig.Style,
  diacriticStyle: KkcConfig.DiacriticStyle,
  autoCommitMode: KkcConfig.AutoCommitMode
) -> KkcConfig {
  var dictDir: URL {
    FileManager.default.urls(for: .applicationSupportDirectory, in: .userDomainMask).first!
      .appendingPathComponent("hazkey", isDirectory: true)
  }

  var zenaiModel: URL {
    dictDir.appendingPathComponent("ggml-model-Q8_0.gguf", isDirectory: false)
  }

  do {
    try FileManager.default.createDirectory(at: dictDir, withIntermediateDirectories: true)
    print("dictDir: \(dictDir)")
  } catch {
    print("Error creating directory: \(error)")
  }

  let options = ConvertRequestOptions.withDefaultDictionary(
    N_best: 9,
    requireJapanesePrediction: false,  // may overwritten
    requireEnglishPrediction: false,  //may overwritten
    keyboardLanguage: .ja_JP,
    typographyLetterCandidate: true,
    unicodeCandidate: true,
    englishCandidateInRoman2KanaInput: true,
    fullWidthRomanCandidate: true,
    halfWidthKanaCandidate: true,
    learningType: .nothing,
    maxMemoryCount: 65536,
    memoryDirectoryURL: dictDir,
    sharedContainerURL: dictDir,
    zenzaiMode: zenzaiEnabled ? .on(weight: zenaiModel) : .off,
    metadata: .init(versionString: "fcitx5-hazkey 0.0.1")
  )
  return KkcConfig(
    convertOptions: options, numberStyle: numberStyle, symbolStyle: symbolStyle,
    periodStyle: periodStyle, commaStyle: commaStyle, spaceStyle: spaceStyle,
    diacriticStyle: diacriticStyle,
    autoCommitMode: autoCommitMode)
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

@MainActor func createCandidateStruct(composingText: ComposingText, options: ConvertRequestOptions)
  -> [UnsafeMutablePointer<
    UnsafeMutablePointer<Int8>?
  >?]
{
  let hiragana = composingText.toHiragana()
  let InvalidLastCharactersForLive: [String.Element] = ["ゃ", "ゅ", "ょ", "ぁ", "ぃ", "ぅ", "ぇ", "ぉ"]

  // one hiragana or two ending with small kana won't convert automatically
  let enableLiveText =
    !((hiragana.count <= 1)
    || (hiragana.count == 2 && InvalidLastCharactersForLive.contains(hiragana.last!)))

  // convert
  let converter = KanaKanjiConverter()
  let converted = converter.requestCandidates(composingText, options: options)

  // create result
  var result: [UnsafeMutablePointer<UnsafeMutablePointer<Int8>?>?] = []
  for candidate in converted.mainResults {

    let candidateRubyLen = candidate.data.reduce(0) { $0 + $1.ruby.count }
    let unconvertedHiragana: String

    var liveTextCompatible: Int
    if candidateRubyLen < hiragana.count {
      // converted is shorter than source
      // get unconverted part of hiragana
      let convertedIndex = hiragana.index(
        hiragana.startIndex, offsetBy: candidateRubyLen)
      unconvertedHiragana = String(hiragana[convertedIndex...])
      liveTextCompatible = 0
    } else if candidateRubyLen == hiragana.count {
      unconvertedHiragana = ""
      liveTextCompatible = enableLiveText ? 1 : 0
    } else {
      unconvertedHiragana = ""
      liveTextCompatible = 0
    }

    // create info list
    var candidatePtrList: [UnsafeMutablePointer<Int8>?] = []
    // about this structure, see header file
    candidatePtrList.append(strdup(candidate.text))  // todo: add description such as [[全]カタカナ]
    candidatePtrList.append(strdup(""))  // todo: usage description like mozc
    candidatePtrList.append(strdup(unconvertedHiragana))
    candidatePtrList.append(strdup(String(candidate.correspondingCount)))
    candidatePtrList.append(strdup(String(liveTextCompatible)))
    for data in candidate.data {
      candidatePtrList.append(strdup(data.word))
      candidatePtrList.append(strdup(String(data.ruby.count)))
    }

    // append to result
    let candidatePtrListPtr = UnsafeMutablePointer<UnsafeMutablePointer<Int8>?>.allocate(
      capacity: candidatePtrList.count + 1)
    candidatePtrListPtr.initialize(from: candidatePtrList, count: candidatePtrList.count)
    candidatePtrListPtr[candidatePtrList.count] = nil
    result.append(candidatePtrListPtr)
  }
  return result
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
  ]
  if reverse {
    let z2h = Dictionary(uniqueKeysWithValues: h2z.map { ($1, $0) })
    return z2h[character] ?? character
  } else {
    return h2z[character] ?? character
  }
}

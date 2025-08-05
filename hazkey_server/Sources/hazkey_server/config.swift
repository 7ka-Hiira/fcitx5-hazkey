import Foundation
import KanaKanjiConverterModule

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

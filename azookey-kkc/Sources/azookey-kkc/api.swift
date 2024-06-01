import Foundation
import KanaKanjiConverterModuleWithDefaultDictionary
import SwiftUtils

///
/// Config
///

public class KkcConfig {
  let convertOptions: ConvertRequestOptions

  init(convertOptions: ConvertRequestOptions) {
    self.convertOptions = convertOptions
  }
}

func genDefaultConfig() -> KkcConfig {
  var dictDir: URL {
    FileManager.default.urls(for: .applicationSupportDirectory, in: .userDomainMask).first!
      .appendingPathComponent("azooKey", isDirectory: true)
      .appendingPathComponent("memory", isDirectory: true)
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
    requireJapanesePrediction: false,
    requireEnglishPrediction: false,
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
    zenzaiMode: .off,
    //zenzaiMode: .on(weight: zenaiModel),
    metadata: .init(versionString: "fcitx5-azooKey 0.0.1")
  )
  return KkcConfig(convertOptions: options)
}

@_silgen_name("kkc_get_config")
@MainActor public func getConfig() -> OpaquePointer? {
  let config = genDefaultConfig()
  let configPtr = Unmanaged.passRetained(config).toOpaque()
  return OpaquePointer(configPtr)
}

@_silgen_name("kkc_free_config")
public func freeConfig(ptr: OpaquePointer?) {
  guard let ptr = ptr else {
    return
  }
  Unmanaged<KkcConfig>.fromOpaque(UnsafeRawPointer(ptr)).release()
}

///
/// ComposingText
///

@_silgen_name("kkc_get_composing_text_instance")
@MainActor public func getComposingTextInstance() -> UnsafeMutablePointer<ComposingText>? {
  let composingText = ComposingText()
  let composingTextPtr = UnsafeMutablePointer<ComposingText>.allocate(capacity: 1)
  composingTextPtr.initialize(to: composingText)
  return composingTextPtr
}

@_silgen_name("kkc_free_composing_text_instance")
public func freeComposingTextInstance(ptr: UnsafeMutablePointer<ComposingText>?) {
  ptr?.deinitialize(count: 1)
  ptr?.deallocate()
}
@_silgen_name("kkc_input_text")
public func inputText(
  composingTextPtr: UnsafeMutablePointer<ComposingText>?, stringPtr: UnsafePointer<Int8>?
) {
  guard let composingTextPtr = composingTextPtr else {
    return
  }
  guard let stringPtr = stringPtr else {
    return
  }

  var string = String(cString: stringPtr)

  let inputStyle: InputStyle
  if string.unicodeScalars.first?.properties.isAlphabetic ?? false {
    inputStyle = .roman2kana
  } else {
    inputStyle = .direct
  }

  if let firstChar = string.unicodeScalars.first, (0x30A0...0x30FF).contains(firstChar.value) {
    string = String(Character(UnicodeScalar(firstChar.value - 96)!))
  }

  switch string {
  case "゛":
    if let lastElem = composingTextPtr.pointee.input.last {
      composingTextPtr.pointee.deleteBackwardFromCursorPosition(count: 1)
      let dakutened = CharacterUtils.dakuten(lastElem.character)
      if dakutened == lastElem.character {
        composingTextPtr.pointee.insertAtCursorPosition(
          String(lastElem.character), inputStyle: lastElem.inputStyle)
      } else {
        string = String(dakutened)
      }
    }
  case "゜":
    if let lastElem = composingTextPtr.pointee.input.last {
      composingTextPtr.pointee.deleteBackwardFromCursorPosition(count: 1)
      let handakutened = CharacterUtils.handakuten(lastElem.character)
      if handakutened == lastElem.character {
        composingTextPtr.pointee.insertAtCursorPosition(
          String(lastElem.character), inputStyle: lastElem.inputStyle)
      } else {
        string = String(handakutened)
      }
    }
  case "-":
    string = "ー"
  case ",":
    string = "、"
  case ".":
    string = "。"
  case " ":
    string = "　"
  default:
    break
  }

  composingTextPtr.pointee.insertAtCursorPosition(string, inputStyle: inputStyle)
}

@_silgen_name("kkc_delete_backward")
public func deleteBackward(composingTextPtr: UnsafeMutablePointer<ComposingText>?) {
  guard let composingTextPtr = composingTextPtr else {
    return
  }
  composingTextPtr.pointee.deleteBackwardFromCursorPosition(count: 1)
}

@_silgen_name("kkc_delete_forward")
public func deleteForward(composingTextPtr: UnsafeMutablePointer<ComposingText>?) {
  guard let composingTextPtr = composingTextPtr else {
    return
  }
  composingTextPtr.pointee.deleteForwardFromCursorPosition(count: 1)
}

@_silgen_name("kkc_complete_prefix")
public func completePrefix(
  composingTextPtr: UnsafeMutablePointer<ComposingText>?, correspondingCount: Int
) {
  guard let composingTextPtr = composingTextPtr else {
    return
  }

  composingTextPtr.pointee.prefixComplete(correspondingCount: correspondingCount)
}

@_silgen_name("kkc_move_cursor")
public func moveCursor(composingTextPtr: UnsafeMutablePointer<ComposingText>?, offset: Int)
  -> Int
{
  guard let composingTextPtr = composingTextPtr else {
    return 0
  }
  return composingTextPtr.pointee.moveCursorFromCursorPosition(count: offset)
}

///
/// ComposingText -> Characters
///

@_silgen_name("kkc_get_composing_hiragana")
public func getComposingHiragana(composingTextPtr: UnsafeMutablePointer<ComposingText>?)
  -> UnsafeMutablePointer<Int8>?
{
  guard let composingTextPtr = composingTextPtr else {
    return nil
  }
  let composingText = composingTextPtr.pointee
  let hiragana = composingText.convertTarget
  return strdup(hiragana)
}

@_silgen_name("kkc_get_composing_katakana_fullwidth")
public func getComposingKatakanaFullwidth(composingTextPtr: UnsafeMutablePointer<ComposingText>?)
  -> UnsafeMutablePointer<Int8>?
{
  guard let composingTextPtr = composingTextPtr else {
    return nil
  }
  let composingText = composingTextPtr.pointee
  let hiragana = composingText.convertTarget
  let katakana = hiragana.applyingTransform(.hiraganaToKatakana, reverse: false) ?? hiragana
  return strdup(katakana)
}

@_silgen_name("kkc_free_composing_katakana_fullwidth")
public func freeComposingKatakanaFullwidth(ptr: UnsafeMutablePointer<Int8>?) {
  free(ptr)
}

@_silgen_name("kkc_get_composing_katakana_halfwidth")
public func getComposingKatakanaHalfwidth(composingTextPtr: UnsafeMutablePointer<ComposingText>?)
  -> UnsafeMutablePointer<Int8>?
{
  guard let composingTextPtr = composingTextPtr else {
    return nil
  }
  let composingText = composingTextPtr.pointee
  let hiragana = composingText.convertTarget
  let katakana = hiragana.applyingTransform(.hiraganaToKatakana, reverse: false) ?? hiragana
  let halfwidthKatakana =
    katakana.applyingTransform(.fullwidthToHalfwidth, reverse: false) ?? katakana
  return strdup(halfwidthKatakana)
}

@_silgen_name("kkc_get_composing_raw_halfwidth")
public func getComposingAlphabetHalfwidth(
  composingTextPtr: UnsafeMutablePointer<ComposingText>?, style: Int
)
  -> UnsafeMutablePointer<Int8>?
{
  guard let composingTextPtr = composingTextPtr else {
    return nil
  }
  let composingText = composingTextPtr.pointee
  let romaji = composingText.input.map {
    var character = $0.character
    if character == "ん" {
      character = "n"
    }
    return String(character)
  }.joined()
  if style == 0 {
    return strdup(romaji.lowercased())
  } else if style == 1 {
    return strdup(romaji.uppercased())
  } else {
    return strdup(romaji.capitalized)
  }
}

@_silgen_name("kkc_get_composing_raw_fullwidth")
public func getComposingAlphabetFullwidth(
  composingTextPtr: UnsafeMutablePointer<ComposingText>?, style: Int
)
  -> UnsafeMutablePointer<Int8>?
{
  guard let composingTextPtr = composingTextPtr else {
    return nil
  }
  let composingText = composingTextPtr.pointee
  let romaji = composingText.input.map {
    var character = $0.character
    if character == "ん" {
      character = "n"
    }
    return String(character)
  }.joined()
  let fullwidthRomaji = romaji.map { character in
    let unicodeScalar = character.unicodeScalars.first!
    if let halfwidthUnicodeScalar = UnicodeScalar(unicodeScalar.value + 0xFEE0) {
      return String(Character(halfwidthUnicodeScalar))
    } else {
      return String(character)
    }
  }.joined()
  if style == 0 {
    return strdup(fullwidthRomaji.lowercased())
  } else if style == 1 {
    return strdup(fullwidthRomaji.uppercased())
  } else {
    return strdup(fullwidthRomaji.capitalized)
  }
}

@_silgen_name("kkc_free_text")
public func freeText(ptr: UnsafeMutablePointer<Int8>?) {
  free(ptr)
}

///
/// Candidates
///

@_silgen_name("kkc_get_candidates")
@MainActor public func getCandidates(
  composingTextPtr: UnsafeMutablePointer<ComposingText>?,
  kkcConfigPtr: OpaquePointer?,
  isPredictMode: Bool?, nBest: Int?
) -> UnsafeMutablePointer<UnsafeMutablePointer<UnsafeMutablePointer<Int8>?>?>? {
  guard let composingTextPtr = composingTextPtr else {
    return nil
  }
  if composingTextPtr.pointee.isEmpty {
    return nil
  }
  let composingText = composingTextPtr.pointee
  let config: KkcConfig
  if let kkcConfigPtr = kkcConfigPtr {
    config = Unmanaged<KkcConfig>.fromOpaque(UnsafeRawPointer(kkcConfigPtr))
      .takeUnretainedValue()
  } else {
    config = genDefaultConfig()
  }
  var options = config.convertOptions
  if nBest != nil {
    options.N_best = nBest!
  }
  if isPredictMode != nil && isPredictMode! {
    options.requireJapanesePrediction = true
    options.requireEnglishPrediction = true
  }

  let hiragana = composingText.convertTarget
  let converter = KanaKanjiConverter()
  let converted = converter.requestCandidates(composingText, options: options)

  var result: [UnsafeMutablePointer<UnsafeMutablePointer<Int8>?>?] = []

  for candidate in converted.mainResults {
    var candidatePtrList: [UnsafeMutablePointer<Int8>?] = []

    let candidateRubyLen = candidate.data.map { $0.ruby }.map { $0.count }.reduce(0, +)
    let unconvertedHiragana: String

    var liveTextCompatible = 0
    let InvalidLastCharactersForLive: [String.Element] = ["ゃ", "ゅ", "ょ", "ぁ", "ぃ", "ぅ", "ぇ", "ぉ"]
    if candidateRubyLen < hiragana.count {
      let convertedIndex = hiragana.index(
        hiragana.startIndex, offsetBy: candidateRubyLen)
      unconvertedHiragana = String(hiragana[convertedIndex...])
    } else if candidateRubyLen == hiragana.count
      && (hiragana.count >= 3
        || (hiragana.count == 2
          && !InvalidLastCharactersForLive.contains(hiragana.last!)))
    {
      unconvertedHiragana = ""
      liveTextCompatible = 1
    } else {
      unconvertedHiragana = ""
    }

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

    let candidatePtrListPtr = UnsafeMutablePointer<UnsafeMutablePointer<Int8>?>.allocate(
      capacity: candidatePtrList.count + 1)
    candidatePtrListPtr.initialize(from: candidatePtrList, count: candidatePtrList.count)
    candidatePtrListPtr[candidatePtrList.count] = nil
    result.append(candidatePtrListPtr)
  }

  let resultPtr = UnsafeMutablePointer<UnsafeMutablePointer<UnsafeMutablePointer<Int8>?>?>
    .allocate(
      capacity: result.count + 1)
  resultPtr.initialize(from: result, count: result.count)
  resultPtr[result.count] = nil

  return resultPtr
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

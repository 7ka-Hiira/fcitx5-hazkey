import Foundation
import KanaKanjiConverterModuleWithDefaultDictionary

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

  // todo: keep this in fcitx5 side
  let options = ConvertRequestOptions.withDefaultDictionary(
    N_best: 1,
    requireJapanesePrediction: false,
    requireEnglishPrediction: false,
    keyboardLanguage: .ja_JP,
    typographyLetterCandidate: true,
    unicodeCandidate: true,
    englishCandidateInRoman2KanaInput: true,
    fullWidthRomanCandidate: false,
    halfWidthKanaCandidate: false,
    learningType: .nothing,
    maxMemoryCount: 1000,
    memoryDirectoryURL: dictDir,
    sharedContainerURL: dictDir,
    zenzaiMode: .off,
    // zenzaiMode: .on(weight: zenaiModel),
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

@_silgen_name("kkc_get_composing_text_instance")
@MainActor public func getComposingTextInstance() -> UnsafeMutablePointer<ComposingText>? {
  let composingText = ComposingText()
  let composingTextPointer = UnsafeMutablePointer<ComposingText>.allocate(capacity: 1)
  composingTextPointer.initialize(to: composingText)
  return composingTextPointer
}

@_silgen_name("kkc_free_composing_text_instance")
public func freeComposingTextInstance(ptr: UnsafeMutablePointer<ComposingText>?) {
  ptr?.deinitialize(count: 1)
  ptr?.deallocate()
}

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

@_silgen_name("kkc_free_composing_hiragana")
public func freeComposingHiragana(ptr: UnsafeMutablePointer<Int8>?) {
  free(ptr)
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
  switch string {
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
  composingTextPtr.pointee.insertAtCursorPosition(string, inputStyle: InputStyle.roman2kana)
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
  print("correspondingCount: \(correspondingCount)")

  composingTextPtr.pointee.prefixComplete(correspondingCount: correspondingCount)
  print("composingTextPtr.pointee: \(composingTextPtr.pointee)")
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

@_silgen_name("kkc_get_candidates")
@MainActor public func getCandidates(
  composingTextPtr: UnsafeMutablePointer<ComposingText>?,
  kkcConfigPtr: OpaquePointer?,
  isPredictMode: Bool?, nBest: Int?
) -> UnsafeMutablePointer<UnsafeMutablePointer<Int8>?>? {
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

  var result: [UnsafeMutablePointer<Int8>?] = []

  for candidate in converted.mainResults {
    let candidateRubyLen = candidate.data.map { $0.ruby }.map { $0.count }.reduce(0, +)
    var preeditHiragana: String

    if candidateRubyLen < hiragana.count {
      let preeditHiraganaIndex = hiragana.index(
        hiragana.startIndex, offsetBy: candidateRubyLen)
      preeditHiragana = String(hiragana[preeditHiraganaIndex...])
    } else {
      preeditHiragana = ""
    }

    // about this structure, see header file
    result.append(strdup(candidate.text))
    result.append(strdup(preeditHiragana))
    result.append(strdup(String(candidate.correspondingCount)))
  }

  let resultPointer = UnsafeMutablePointer<UnsafeMutablePointer<Int8>?>.allocate(
    capacity: result.count + 1)
  resultPointer.initialize(from: result, count: result.count)
  resultPointer[result.count] = nil

  return resultPointer
}

@_silgen_name("kkc_free_candidates")
public func freeCandidates(ptr: UnsafeMutablePointer<UnsafeMutablePointer<Int8>?>?) {
  guard let ptr = ptr else {
    return
  }
  var index = 0
  while let p = ptr[index] {
    free(p)
    index += 1
  }
  ptr.deallocate()
}

@_silgen_name("kkc_free_first_candidate")
public func freeFirstCandidate(ptr: UnsafeMutablePointer<Int8>?) {
  free(ptr)
}

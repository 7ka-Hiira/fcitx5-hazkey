import Foundation
import KanaKanjiConverterModuleWithDefaultDictionary

public struct KkcConfig {
  let convertOptions: ConvertRequestOptions
}

@_silgen_name("kkc_get_config")
@MainActor public func getConfig() -> UnsafeMutablePointer<KkcConfig>? {
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
    N_best: 4,
    requireJapanesePrediction: false,
    requireEnglishPrediction: false,
    keyboardLanguage: .ja_JP,
    typographyLetterCandidate: true,
    unicodeCandidate: true,
    englishCandidateInRoman2KanaInput: true,
    fullWidthRomanCandidate: false,
    halfWidthKanaCandidate: false,
    learningType: .nothing,
    maxMemoryCount: 100,
    memoryDirectoryURL: dictDir,
    sharedContainerURL: dictDir,
    zenzaiMode: .off,
    //zenzaiMode: .on(weight: zenaiModel),
    metadata: .init(versionString: "Your app version X")
  )
  let config = KkcConfig(convertOptions: options)
  let configPtr = UnsafeMutablePointer<KkcConfig>.allocate(capacity: 1)
  configPtr.initialize(to: config)
  return configPtr
}

@_silgen_name("kkc_free_config")
public func freeConfig(ptr: UnsafeMutablePointer<KkcConfig>?) {
  ptr?.deinitialize(count: 1)
  ptr?.deallocate()
}

@_silgen_name("kkc_get_composing_text")
@MainActor public func getComposingText() -> UnsafeMutablePointer<ComposingText>? {
  let composingText = ComposingText()
  let composingTextPointer = UnsafeMutablePointer<ComposingText>.allocate(capacity: 1)
  composingTextPointer.initialize(to: composingText)
  return composingTextPointer
}

@_silgen_name("kkc_free_composing_text")
public func freeComposingText(ptr: UnsafeMutablePointer<ComposingText>?) {
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
  switch string {
  case "minus":
    string = "ー"
  case "comma":
    string = "、"
  case "period":
    string = "。"
  case "space":
    string = " "
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
  composingTextPtr: UnsafeMutablePointer<ComposingText>?, kkcConfigPtr: UnsafePointer<KkcConfig>?
) -> UnsafeMutablePointer<UnsafeMutablePointer<Int8>?>? {
  guard let composingTextPtr = composingTextPtr else {
    return nil
  }
  guard let kkcConfigPointer = kkcConfigPtr else {
    return nil
  }

  let composingText = composingTextPtr.pointee
  let options = kkcConfigPointer.pointee.convertOptions

  let converter = KanaKanjiConverter()
  let converted = converter.requestCandidates(composingText, options: options)
  let result = converted.mainResults
    .map { $0.text }
    .map { strdup($0) }
    .map { UnsafeMutablePointer(mutating: $0) }
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

@_silgen_name("kkc_get_first_candidate")
@MainActor public func getFirstCandidate(
  composingTextPtr: UnsafeMutablePointer<ComposingText>?, kkcConfigPtr: UnsafePointer<KkcConfig>?
) -> UnsafeMutablePointer<Int8>? {
  guard let composingTextPtr = composingTextPtr else {
    return nil
  }
  guard let kkcConfigPointer = kkcConfigPtr else {
    return nil
  }

  let composingText = composingTextPtr.pointee
  var options = kkcConfigPointer.pointee.convertOptions

  options.N_best = 1

  let converter = KanaKanjiConverter()
  let converted = converter.requestCandidates(composingText, options: options)
  let result = converted.mainResults.first?.text
  return strdup(result ?? "")
}

@_silgen_name("kkc_free_first_candidate")
public func freeFirstCandidate(ptr: UnsafeMutablePointer<Int8>?) {
  free(ptr)
}

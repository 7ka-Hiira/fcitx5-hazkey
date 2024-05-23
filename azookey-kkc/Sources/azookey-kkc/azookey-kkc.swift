import Foundation
import KanaKanjiConverterModuleWithDefaultDictionary

public struct KkcConfig {
    let convert_options: ConvertRequestOptions
}

@_silgen_name("kkc_get_config")
@MainActor public func kkc_get_config() -> UnsafeMutablePointer<KkcConfig>? {
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
    let config = KkcConfig(convert_options: options)
    let config_ptr = UnsafeMutablePointer<KkcConfig>.allocate(capacity: 1)
    config_ptr.initialize(to: config)
    return config_ptr
}

@_silgen_name("kkc_free_config")
public func kkc_free_config(ptr: UnsafeMutablePointer<KkcConfig>?) {
    ptr?.deinitialize(count: 1)
    ptr?.deallocate()
}

@_silgen_name("kkc_convert")
@MainActor public func kkc_convert(stringPtr: UnsafePointer<Int8>?, kkcConfigPtr: UnsafePointer<KkcConfig>?) -> UnsafeMutablePointer<UnsafeMutablePointer<Int8>?>? {

    guard let stringPtr = stringPtr else {
        return nil
    }
    guard let kkcConfigPointer = kkcConfigPtr else {
        return nil
    }

    let kana = String(cString: stringPtr)
    let options = kkcConfigPointer.pointee.convert_options

    let converter = KanaKanjiConverter()
    var c = ComposingText()
    c.insertAtCursorPosition(kana, inputStyle: InputStyle.direct)
    let converted = converter.requestCandidates(c, options: options)
    let result = converted.mainResults
        .map { $0.text }
        .map { strdup($0) }
        .map { UnsafeMutablePointer(mutating: $0) }
    let resultPointer = UnsafeMutablePointer<UnsafeMutablePointer<Int8>?>.allocate(capacity: result.count + 1)
    resultPointer.initialize(from: result, count: result.count)
    resultPointer[result.count] = nil
    return resultPointer
}

@_silgen_name("kkc_free_convert")
public func kkc_free_convert(ptr: UnsafeMutablePointer<UnsafeMutablePointer<Int8>?>?) {
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
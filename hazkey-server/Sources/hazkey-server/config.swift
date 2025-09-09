import Foundation
import KanaKanjiConverterModule
import SwiftProtobuf

let KEYMAP_FILE_SIZE_LIMIT = 1024 * 1024  //1MB
let TABLE_FILE_SIZE_LIMIT = 1024 * 1024  //1MB

let builtInKeymaps = [
    "Fullwidth Period",
    "Fullwidth Comma",
    "Japanese Symbol",
    "Fullwidth Symbol",
    "Fullwidth Number",
].map { name in
    Hazkey_Config_Keymap.with {
        $0.name = name
        $0.isBuiltIn = true
        $0.filename = name
    }
}

let builtInInputTables = [
    "Romaji",
    "Kana",
].map { name in
    Hazkey_Config_InputTable.with {
        $0.name = name
        $0.isBuiltIn = true
        $0.filename = name
    }
}

func getCurrentConfig() -> Hazkey_ResponseEnvelope {
    let profiles: [Hazkey_Config_Profile]
    do {
        profiles = try loadConfig()
    } catch {
        return Hazkey_ResponseEnvelope.with {
            $0.status = .failed
            $0.errorMessage = "\(error)"
        }
    }

    let userKeymapDir = getConfigDirectory().appendingPathComponent(
        "keymap", isDirectory: true
    )
    var keymaps = builtInKeymaps
    do {
        try FileManager.default.createDirectory(
            at: userKeymapDir, withIntermediateDirectories: true)
        let fileURLs = try FileManager.default.contentsOfDirectory(
            at: userKeymapDir,
            includingPropertiesForKeys: [.fileSizeKey],
            options: [.skipsHiddenFiles]
        )

        let keymapFiles = try fileURLs.filter { url in
            guard url.pathExtension.lowercased() == "tsv" else { return false }
            let attrs = try url.resourceValues(forKeys: [.fileSizeKey])
            if let size = attrs.fileSize {
                return size < KEYMAP_FILE_SIZE_LIMIT
            }
            return false
        }

        for file in keymapFiles {
            keymaps.append(
                Hazkey_Config_Keymap.with {
                    $0.name = file.deletingPathExtension().lastPathComponent
                    $0.isBuiltIn = false
                    $0.filename = file.lastPathComponent
                })
        }
    } catch {
        return Hazkey_ResponseEnvelope.with {
            $0.status = .failed
            $0.errorMessage = "Failed to get user keymap files: \(error)"
        }
    }

    let userInputTableDir = getConfigDirectory().appendingPathComponent(
        "table", isDirectory: true
    )
    var inputTables = builtInInputTables
    do {
        try FileManager.default.createDirectory(
            at: userInputTableDir, withIntermediateDirectories: true)
        let fileURLs = try FileManager.default.contentsOfDirectory(
            at: userInputTableDir,
            includingPropertiesForKeys: [.fileSizeKey],
            options: [.skipsHiddenFiles]
        )

        let inputTableFiles = try fileURLs.filter { url in
            guard url.pathExtension.lowercased() == "tsv" else { return false }
            let attrs = try url.resourceValues(forKeys: [.fileSizeKey])
            if let size = attrs.fileSize {
                return size < TABLE_FILE_SIZE_LIMIT
            }
            return false
        }

        for file in inputTableFiles {
            inputTables.append(
                Hazkey_Config_InputTable.with {
                    $0.name = file.deletingPathExtension().lastPathComponent
                    $0.isBuiltIn = false
                    $0.filename = file.lastPathComponent
                })
        }
    } catch {
        return Hazkey_ResponseEnvelope.with {
            $0.status = .failed
            $0.errorMessage = "Failed to get user input table files: \(error)"
        }
    }

    let currentConfig = Hazkey_Config_CurrentConfig.with {
        $0.fileHashes = []
        $0.isZenzaiAvailable = zenzaiAvailable
        $0.xdgConfigHomePath = getConfigDirectory().absoluteString
        $0.availableKeymaps = keymaps
        $0.availableTables = inputTables
        $0.profiles = profiles
    }
    return Hazkey_ResponseEnvelope.with {
        $0.status = .success
        $0.currentConfig = currentConfig
    }
}

@MainActor
func setCurrentConfig(
    _ hashes: [Hazkey_Config_FileHash],
    _ profiles: [Hazkey_Config_Profile]
) -> Hazkey_ResponseEnvelope {
    do {
        try saveConfig(profiles)
    } catch {
        return Hazkey_ResponseEnvelope.with {
            $0.status = .failed
            $0.errorMessage = "\(error)"
        }
    }

    return Hazkey_ResponseEnvelope.with {
        $0.status = .success
    }
}

func genDefaultConfig() -> Hazkey_Config_Profile {
    var newConf = Hazkey_Config_Profile.init()
    newConf.profileName = "Default"
    newConf.autoConvertMode =
        Hazkey_Config_Profile.AutoConvertMode.autoConvertForMultipleChars
    newConf.auxTextMode = Hazkey_Config_Profile.AuxTextMode.auxTextShowWhenCursorNotAtEnd
    newConf.suggestionListMode =
        Hazkey_Config_Profile.SuggestionListMode.suggestionListDisabled
    newConf.numSuggestions = 4
    newConf.useRichSuggestion = false
    newConf.numCandidatesPerPage = 9
    newConf.useRichCandidates = false
    newConf.useInputHistory = true
    newConf.stopStoreNewHistory = false
    newConf.enabledKeymaps = [
        Hazkey_Config_Profile.EnabledKeymap.with {
            $0.name = "Fullwidth Number"
            $0.isBuiltIn = true
            $0.filename = "Fullwidth Number"
        },
        Hazkey_Config_Profile.EnabledKeymap.with {
            $0.name = "Japanese Symbol"
            $0.isBuiltIn = true
            $0.filename = "Japanese Symbol"
        },
        Hazkey_Config_Profile.EnabledKeymap.with {
            $0.name = "Fullwidth Symbol"
            $0.isBuiltIn = true
            $0.filename = "Fullwidth Symbol"
        },
    ]
    newConf.enabledTables = [
        Hazkey_Config_Profile.EnabledInputTable.with {
            $0.name = "Romaji"
            $0.isBuiltIn = true
            $0.filename = "Romaji"
        }
    ]
    newConf.zenzaiEnable = true
    newConf.zenzaiInferLimit = 10
    newConf.zenzaiContextualMode = true
    newConf.zenzaiVersionConfig.v3 = Hazkey_Config_Profile.ZenzaiVersionConfig.V3.init()
    return newConf
}

@MainActor
func saveConfig(_ profiles: [Hazkey_Config_Profile]) throws {
    let configDir = getConfigDirectory()
    let configPath = configDir.appendingPathComponent("config.json")

    // Create directory if it doesn't exist
    try FileManager.default.createDirectory(
        at: configDir, withIntermediateDirectories: true, attributes: nil)

    // Convert each protobuf to JSON string and then combine into array
    var jsonObjects: [Any] = []
    var encodeOptions = JSONEncodingOptions()
    encodeOptions.alwaysPrintEnumsAsInts = true
    encodeOptions.useDeterministicOrdering = true
    for profile in profiles {
        let jsonData = try profile.jsonUTF8Data(options: encodeOptions)
        let jsonObject = try JSONSerialization.jsonObject(with: jsonData, options: [])
        jsonObjects.append(jsonObject)
    }

    // Convert array to JSON data
    let jsonData = try JSONSerialization.data(
        withJSONObject: jsonObjects, options: [.prettyPrinted])

    // Write to file
    try jsonData.write(to: configPath)

    NSLog("Config saved to: \(configPath.path)")
    keymap = loadKeymap()
    let newTableName = UUID().uuidString
    loadInputTable(tableName: newTableName)
    currentTableName = newTableName
}

func loadConfig() throws -> [Hazkey_Config_Profile] {
    let configDir = getConfigDirectory()
    let configPath = configDir.appendingPathComponent("config.json")

    // Check if config file exists
    guard FileManager.default.fileExists(atPath: configPath.path) else {
        NSLog("Config file does not exist at: \(configPath.path), returning empty config")
        return [genDefaultConfig()]
    }

    // Read file contents
    let jsonData = try Data(contentsOf: configPath)

    // Parse JSON array
    let jsonArray =
        try JSONSerialization.jsonObject(with: jsonData, options: []) as! [[String: Any]]

    var configs: [Hazkey_Config_Profile] = []
    var decodeOptions = JSONDecodingOptions()
    decodeOptions.ignoreUnknownFields = true
    for jsonObject in jsonArray {
        let jsonObjectData = try JSONSerialization.data(withJSONObject: jsonObject, options: [])
        let config = try Hazkey_Config_Profile(jsonUTF8Data: jsonObjectData, options: decodeOptions)
        configs.append(config)
    }

    if configs.count == 0 {
        NSLog("Loaded empty config. returning default config...")
        return [genDefaultConfig()]
    }

    NSLog("Config loaded from: \(configPath.path)")
    return configs
}

func getConfigDirectory() -> URL {
    if let xdgConfigHome = ProcessInfo.processInfo.environment["XDG_CONFIG_HOME"],
        !xdgConfigHome.isEmpty
    {
        return URL(fileURLWithPath: xdgConfigHome).appendingPathComponent("hazkey")
    }

    // Fallback to ~/.config/hazkey
    let homeDir = FileManager.default.homeDirectoryForCurrentUser
    return homeDir.appendingPathComponent(".config").appendingPathComponent("hazkey")
}

@MainActor
func genZenzaiMode(leftContext: String) -> ConvertRequestOptions.ZenzaiMode {
    if zenzaiAvailable && currentProfile.zenzaiEnable {
        return ConvertRequestOptions.ZenzaiMode.on(
            weight: systemResourceDir.appendingPathComponent("zenzai.gguf", isDirectory: false),
            inferenceLimit: Int(currentProfile.zenzaiInferLimit),
            requestRichCandidates: currentProfile.useRichCandidates,
            personalizationMode: nil,
            versionDependentMode: {
                return switch currentProfile.zenzaiVersionConfig.version {
                case .v1:
                    .v1
                case .v2(let versionConfig):
                    .v2(
                        ConvertRequestOptions.ZenzaiV2DependentMode.init(
                            profile: versionConfig.profile,
                            leftSideContext: currentProfile.zenzaiContextualMode ? leftContext : nil
                        ))
                case .v3(let versionConfig):
                    .v3(
                        ConvertRequestOptions.ZenzaiV3DependentMode.init(
                            profile: versionConfig.profile,
                            topic: versionConfig.topic,
                            style: versionConfig.style,
                            preference: versionConfig.style,
                            leftSideContext: currentProfile.zenzaiContextualMode ? leftContext : nil
                        ))
                default:
                    .v1
                }
            }()
        )
    } else {
        return ConvertRequestOptions.ZenzaiMode.off
    }
}

@MainActor
func genBaseConvertRequestOptions() -> ConvertRequestOptions {
    var userDataDir: URL {
        FileManager.default.urls(for: .applicationSupportDirectory, in: .userDomainMask).first!
            .appendingPathComponent("hazkey", isDirectory: true)
    }

    let learningType =
        switch (currentProfile.useInputHistory, currentProfile.stopStoreNewHistory) {
        case (true, false):
            LearningType.inputAndOutput
        case (true, true):
            LearningType.onlyOutput
        default:
            LearningType.nothing
        }

    let specialCandidateProviders: [any SpecialCandidateProvider] = {
        let mode = currentProfile.specialConversionMode
        let providers: [SpecialCandidateProvider?] = [
            mode.commaSeparatedNumber ? CommaSeparatedNumberSpecialCandidateProvider() : nil,
            mode.calendar ? CalendarSpecialCandidateProvider() : nil,
            mode.hazkeyVersion ? VersionSpecialCandidateProvider() : nil,
            mode.mailDomain ? EmailAddressSpecialCandidateProvider() : nil,
            mode.romanTypography ? TypographySpecialCandidateProvider() : nil,
            mode.time ? TimeExpressionSpecialCandidateProvider() : nil,
            mode.unicodeCodepoint ? UnicodeSpecialCandidateProvider() : nil,
        ]
        return providers.compactMap { $0 }
    }()

    let zenzaiMode = genZenzaiMode(leftContext: "")

    return ConvertRequestOptions.init(
        N_best: Int(currentProfile.numCandidatesPerPage),
        needTypoCorrection: false,
        requireJapanesePrediction: false,
        requireEnglishPrediction: false,
        keyboardLanguage: .none,
        englishCandidateInRoman2KanaInput: false,
        fullWidthRomanCandidate: true,
        halfWidthKanaCandidate: true,
        learningType: learningType,
        maxMemoryCount: 65536,
        shouldResetMemory: false,
        memoryDirectoryURL: userDataDir.appendingPathComponent(
            "memory", isDirectory: true),
        sharedContainerURL: userDataDir.appendingPathComponent(
            "shared", isDirectory: true),
        textReplacer: .empty,
        specialCandidateProviders: specialCandidateProviders,
        zenzaiMode: zenzaiMode,
        preloadDictionary: false,
        metadata: ConvertRequestOptions.Metadata.init(versionString: "Hazkey 0.0.9")
    )
}

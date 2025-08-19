import Foundation
import KanaKanjiConverterModule
import SwiftProtobuf

func getCurrentConfig() -> Hazkey_ResponseEnvelope {
    let profiles = loadConfig()

    return Hazkey_ResponseEnvelope.with {
        $0.status = .success
        $0.currentConfig.configs = profiles
    }
}

func setConfig(
    _ hashes: [Hazkey_Config_FileHash],
    _ inputTableOperations: [Hazkey_Config_Table_EditOperation],
    _ config: [Hazkey_Config_ConfigProfile]
) -> Hazkey_ResponseEnvelope {

    saveConfig(config: config)

    return Hazkey_ResponseEnvelope.with {
        $0.status = .success
    }
}

func genDefaultConfig() -> Hazkey_Config_ConfigProfile {
    var newConf = Hazkey_Config_ConfigProfile.init()
    newConf.profileName = "Default"
    newConf.autoConvertMode =
        Hazkey_Config_ConfigProfile.AutoConvertMode.autoConvertForMultipleChars
    newConf.auxTextMode = Hazkey_Config_ConfigProfile.AuxTextMode.auxTextShowAlways
    newConf.suggestionListMode =
        Hazkey_Config_ConfigProfile.SuggestionListMode.suggestionListDisabled
    newConf.numSuggestions = 4
    newConf.useRichSuggestion = false
    newConf.numCandidatesPerPage = 9
    newConf.useRichCandidates = false
    newConf.useInputHistory = true
    newConf.stopStoreNewHistory = false
    newConf.zenzaiEnable = true
    newConf.zenzaiInferLimit = 1
    newConf.zenzaiContextualMode = true
    return newConf
}

func saveConfig(config: [Hazkey_Config_ConfigProfile]) {
    do {
        let configDir = getConfigDirectory()
        let configPath = configDir.appendingPathComponent("config.json")

        // Create directory if it doesn't exist
        try FileManager.default.createDirectory(
            at: configDir, withIntermediateDirectories: true, attributes: nil)

        // Convert each protobuf to JSON string and then combine into array
        var jsonObjects: [Any] = []
        for profile in config {
            let jsonString = try profile.jsonString()
            let jsonData = jsonString.data(using: .utf8)!
            let jsonObject = try JSONSerialization.jsonObject(with: jsonData, options: [])
            jsonObjects.append(jsonObject)
        }

        // Convert array to JSON data
        let jsonData = try JSONSerialization.data(
            withJSONObject: jsonObjects, options: [.prettyPrinted])

        // Write to file
        try jsonData.write(to: configPath)

        NSLog("Config saved to: \(configPath.path)")
    } catch {
        NSLog("Failed to save config: \(error)")
    }
}

func loadConfig() -> [Hazkey_Config_ConfigProfile] {
    do {
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

        var configs: [Hazkey_Config_ConfigProfile] = []
        for jsonObject in jsonArray {
            let jsonObjectData = try JSONSerialization.data(withJSONObject: jsonObject, options: [])
            let config = try Hazkey_Config_ConfigProfile(jsonUTF8Data: jsonObjectData)
            configs.append(config)
        }

        if configs.count == 0 {
            NSLog("Loaded empty config. returning default config...")
            return [genDefaultConfig()]
        }

        NSLog("Config loaded from: \(configPath.path)")
        return configs
    } catch {
        NSLog("Failed to load config: \(error), returning default config")
        return [genDefaultConfig()]
    }
}

// replace with FileManager.default.urls(for: .applicationSupportDirectory, in: .userDomainMask).first!?
func getConfigDirectory() -> URL {
    let homeDir = FileManager.default.homeDirectoryForCurrentUser

    // Check for XDG_CONFIG_HOME environment variable
    if let xdgConfigHome = ProcessInfo.processInfo.environment["XDG_CONFIG_HOME"],
        !xdgConfigHome.isEmpty
    {
        return URL(fileURLWithPath: xdgConfigHome).appendingPathComponent("hazkey")
    }

    // Fallback to ~/.config/hazkey
    return homeDir.appendingPathComponent(".config").appendingPathComponent("hazkey")
}

@MainActor
func genZenzaiMode(leftContext: String) -> ConvertRequestOptions.ZenzaiMode {
    var systemResourceDir: URL {
        URL(fileURLWithPath: systemResourcePath, isDirectory: true)
    }

    if currentProfile.zenzaiEnable {
        return ConvertRequestOptions.ZenzaiMode.on(
            weight: systemResourceDir.appendingPathComponent("zenz-v3.gguf", isDirectory: false),
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

    var systemResourceDir: URL {
        URL(fileURLWithPath: systemResourcePath, isDirectory: true)
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
            mode.calender ? CalendarSpecialCandidateProvider() : nil,
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

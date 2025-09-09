import Foundation
import KanaKanjiConverterModule
import OrderedCollections

@MainActor
func loadKeymap() -> Keymap {
    var maps: Keymap = [:]
    outer: for enabledKeymap in currentProfile.enabledKeymaps.reversed() {
        var newKeymapRule: Keymap
        if enabledKeymap.isBuiltIn {
            switch enabledKeymap.filename {
            case "Fullwidth Period":
                newKeymapRule = fullwidthPeriodMap
            case "Fullwidth Comma":
                newKeymapRule = fullwidthCommaMap
            case "Japanese Symbol":
                newKeymapRule = fullwidthJapaneseSymbolMap
            case "Fullwidth Symbol":
                newKeymapRule = fullwidthSymbolMap
            case "Fullwidth Number":
                newKeymapRule = fullwidthNumberMap
            default:
                NSLog("Unknown built-in keymap: \(enabledKeymap.name)")
                continue outer
            }
        } else {
            // load custom keymap
            let customKeymapFile = getConfigDirectory().appendingPathComponent(
                "keymap", isDirectory: true
            ).appendingPathComponent(enabledKeymap.filename, isDirectory: false)
            do {
                let lines = try String(contentsOf: customKeymapFile, encoding: .utf8)
                    .split(separator: "\n")
                    .map { $0.split(separator: "\t") }
                newKeymapRule = [:]
                inner: for cols in lines {
                    guard let key = cols[0].first else { continue inner }
                    switch cols.count {
                    case 1:
                        newKeymapRule[key] = nil
                    case 2:
                        newKeymapRule[key] = (cols[1].first!, nil)
                    case 3...:
                        newKeymapRule[key] = (cols[1].first!, cols[2].first)
                    default:
                        NSLog("Unknown columns count: \(cols.count)")
                        continue inner
                    }
                }
            } catch {
                NSLog(
                    "Failed to load custom keymap \(enabledKeymap.name): \(error)"
                )
                continue outer
            }
        }
        maps.merge(newKeymapRule) { (_, second) in second }
    }

    return maps
}

@MainActor
func loadInputTable(tableName: String) {
    var tables: [InputTable] = []
    outer: for enabledTable in currentProfile.enabledTables.reversed() {
        let tableToAdd: InputTable
        if enabledTable.isBuiltIn {
            switch enabledTable.filename {
            case "Romaji":
                tableToAdd = InputTable.defaultRomanToKana
            case "Kana":
                tableToAdd = kanaTable
            default:
                debugLog("Unknown built-in input table: \(enabledTable.name)")
                continue outer
            }
        } else {
            // load custom table
            let customTableFile = getConfigDirectory().appendingPathComponent(
                "table", isDirectory: true
            ).appendingPathComponent(enabledTable.filename, isDirectory: false)
            do {
                tableToAdd = try InputStyleManager.loadTable(from: customTableFile)
            } catch {
                NSLog("Failed to load custom table \(enabledTable.name)Q \(error)")
                continue outer
            }
        }
        tables.append(tableToAdd)
    }

    let inputTable = InputTable(tables: tables, order: InputTable.Ordering.lastInputWins)
    InputStyleManager.registerInputStyle(table: inputTable, for: tableName)
}

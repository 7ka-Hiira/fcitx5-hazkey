import Foundation

/// User-defined word entry (reading -> word).
struct UserDictionaryEntry {
    let reading: String
    let word: String
    let comment: String
}

/// Loads and caches the user dictionary file.
///
/// File format (TSV, UTF-8):
///   reading<TAB>word[<TAB>comment]
/// Lines starting with '#' and empty lines are ignored.
/// Reading is normalized to hiragana for matching.
class UserDictionary {
    private var entries: [UserDictionaryEntry] = []
    private var lastModified: Date? = nil
    private var lastLoadedPath: String = ""

    /// Default path: $XDG_CONFIG_HOME/hazkey/user_dictionary.tsv
    static func defaultPath() -> URL {
        return HazkeyServerConfig.getConfigDirectory()
            .appendingPathComponent("user_dictionary.tsv", isDirectory: false)
    }

    /// Reload from disk if the file's mtime changed (or never loaded).
    /// Safe to call frequently.
    func reloadIfNeeded() {
        let url = Self.defaultPath()
        let fm = FileManager.default
        guard fm.fileExists(atPath: url.path) else {
            if !entries.isEmpty || lastModified != nil {
                entries = []
                lastModified = nil
            }
            lastLoadedPath = url.path
            return
        }
        do {
            let attrs = try fm.attributesOfItem(atPath: url.path)
            let mtime = attrs[.modificationDate] as? Date
            if lastLoadedPath == url.path, let last = lastModified, let cur = mtime, last == cur {
                return
            }
            let content = try String(contentsOf: url, encoding: .utf8)
            var newEntries: [UserDictionaryEntry] = []
            for rawLine in content.split(whereSeparator: { $0 == "\n" || $0 == "\r" }) {
                let line = String(rawLine)
                if line.isEmpty || line.hasPrefix("#") { continue }
                let cols = line.split(separator: "\t", omittingEmptySubsequences: false).map(String.init)
                guard cols.count >= 2 else { continue }
                let reading = cols[0].trimmingCharacters(in: .whitespaces)
                    .precomposedStringWithCanonicalMapping
                let word = cols[1]
                let comment = cols.count >= 3 ? cols[2] : ""
                if reading.isEmpty || word.isEmpty { continue }
                newEntries.append(
                    UserDictionaryEntry(reading: reading, word: word, comment: comment))
            }
            entries = newEntries
            lastModified = mtime
            lastLoadedPath = url.path
            NSLog("Loaded \(entries.count) user dictionary entries from \(url.path)")
        } catch {
            NSLog("Failed to load user dictionary: \(error.localizedDescription)")
        }
    }

    /// Returns entries whose reading exactly equals `hiragana`.
    func exactMatches(hiragana: String) -> [UserDictionaryEntry] {
        if hiragana.isEmpty { return [] }
        return entries.filter { $0.reading == hiragana }
    }

    /// Total entry count (for diagnostics).
    var count: Int { entries.count }
}

import Foundation
import SwiftGlibc
import XCTest

@testable import hazkey_server

private enum UserDictTestError: Error {
  case missingModificationDate
}

// Pure unit tests for UserDictionary (TSV parsing + exact matching).
//
// XDG_CONFIG_HOME is redirected to a temporary directory so that the tests
// never touch the real ~/.config/hazkey/user_dictionary.tsv. The environment
// variable is restored in tearDown because integration tests in this target
// share the process.
final class UserDictionaryTests: XCTestCase {
  private var originalConfigHome: String?
  private var tempRoot = URL(fileURLWithPath: "")

  override func setUpWithError() throws {
    try super.setUpWithError()
    originalConfigHome = ProcessInfo.processInfo.environment["XDG_CONFIG_HOME"]
    tempRoot = FileManager.default.temporaryDirectory
      .appendingPathComponent("hazkey-userdict-tests-\(UUID().uuidString)")
    try FileManager.default.createDirectory(
      at: tempRoot.appendingPathComponent("hazkey"),
      withIntermediateDirectories: true)
    setenv("XDG_CONFIG_HOME", tempRoot.path, 1)

    XCTAssertEqual(
      UserDictionary.defaultPath().path,
      dictionaryPath.path,
      "UserDictionary.defaultPath() must resolve under the redirected XDG_CONFIG_HOME")
  }

  override func tearDownWithError() throws {
    if let original = originalConfigHome {
      setenv("XDG_CONFIG_HOME", original, 1)
    } else {
      unsetenv("XDG_CONFIG_HOME")
    }
    try? FileManager.default.removeItem(at: tempRoot)
    try super.tearDownWithError()
  }

  private var dictionaryPath: URL {
    tempRoot.appendingPathComponent("hazkey/user_dictionary.tsv")
  }

  private func writeDictionary(_ content: String) throws {
    try content.write(to: dictionaryPath, atomically: true, encoding: .utf8)
  }

  // Explicitly bumps the mtime so reloadIfNeeded() observes the change
  // regardless of filesystem timestamp granularity.
  private func bumpModificationDate() throws {
    let attrs = try FileManager.default.attributesOfItem(atPath: dictionaryPath.path)
    guard let mtime = attrs[.modificationDate] as? Date else {
      throw UserDictTestError.missingModificationDate
    }
    try FileManager.default.setAttributes(
      [.modificationDate: mtime.addingTimeInterval(2)],
      ofItemAtPath: dictionaryPath.path)
  }

  func testNoFileStartsEmpty() {
    let dictionary = UserDictionary()
    dictionary.reloadIfNeeded()
    XCTAssertEqual(dictionary.count, 0)
    XCTAssertTrue(dictionary.exactMatches(hiragana: "めーる").isEmpty)
  }

  func testParsesValidEntries() throws {
    try writeDictionary(
      """
      # reading<TAB>word<TAB>comment
      めーる\tmail@example.com\twork
      ねこ\t🐱
      """)
    let dictionary = UserDictionary()
    dictionary.reloadIfNeeded()
    XCTAssertEqual(dictionary.count, 2)

    XCTAssertEqual(
      dictionary.exactMatches(hiragana: "めーる").map(\.word),
      ["mail@example.com"])
    XCTAssertEqual(
      dictionary.exactMatches(hiragana: "めーる").first?.comment, "work")
    XCTAssertEqual(
      dictionary.exactMatches(hiragana: "ねこ").map(\.word),
      ["🐱"])
  }

  func testIgnoresCommentsBlankAndMalformedLines() throws {
    try writeDictionary(
      """

      # a comment line
      onlyonecolumn
      \t空よみ
      くうご\t
      \t\t
      ただしい\tせいこう
      """)
    let dictionary = UserDictionary()
    dictionary.reloadIfNeeded()
    XCTAssertEqual(dictionary.count, 1)
    XCTAssertEqual(
      dictionary.exactMatches(hiragana: "ただしい").map(\.word),
      ["せいこう"])
  }

  func testTrimsReadingWhitespace() throws {
    try writeDictionary("  めーる  \tmail@example.com")
    let dictionary = UserDictionary()
    dictionary.reloadIfNeeded()
    XCTAssertEqual(
      dictionary.exactMatches(hiragana: "めーる").map(\.word),
      ["mail@example.com"])
  }

  func testExactMatchOnly() throws {
    try writeDictionary("めーる\tmail@example.com")
    let dictionary = UserDictionary()
    dictionary.reloadIfNeeded()
    XCTAssertTrue(dictionary.exactMatches(hiragana: "め").isEmpty)
    XCTAssertTrue(dictionary.exactMatches(hiragana: "めーるん").isEmpty)
    XCTAssertTrue(dictionary.exactMatches(hiragana: "").isEmpty)
  }

  func testReadingNormalizedToNFC() throws {
    // "が" written in NFD (か + U+3099 combining voiced sound mark).
    try writeDictionary("\u{304B}\u{3099}\tがぞう")
    let dictionary = UserDictionary()
    dictionary.reloadIfNeeded()
    XCTAssertEqual(
      dictionary.exactMatches(hiragana: "が").map(\.word),
      ["がぞう"])
  }

  func testReloadsWhenFileChanges() throws {
    try writeDictionary("めーる\tmail@example.com")
    let dictionary = UserDictionary()
    dictionary.reloadIfNeeded()
    XCTAssertEqual(dictionary.count, 1)

    try writeDictionary(
      """
      めーる\tmail@example.com
      でんわ\t☎️
      """)
    try bumpModificationDate()
    dictionary.reloadIfNeeded()
    XCTAssertEqual(dictionary.count, 2)
    XCTAssertEqual(
      dictionary.exactMatches(hiragana: "でんわ").map(\.word),
      ["☎️"])
  }

  func testUnchangedMtimeKeepsCachedEntries() throws {
    try writeDictionary("めーる\tmail@example.com")
    // Normalize the mtime to whole seconds so that restoring this exact value
    // round-trips through the filesystem regardless of timestamp precision.
    let attrs = try FileManager.default.attributesOfItem(atPath: dictionaryPath.path)
    guard let mtime = attrs[.modificationDate] as? Date else {
      throw UserDictTestError.missingModificationDate
    }
    let wholeSecondMtime = Date(timeIntervalSince1970: mtime.timeIntervalSince1970.rounded(.down))
    try FileManager.default.setAttributes(
      [.modificationDate: wholeSecondMtime], ofItemAtPath: dictionaryPath.path)

    let dictionary = UserDictionary()
    dictionary.reloadIfNeeded()
    XCTAssertEqual(dictionary.count, 1)

    // Overwrite the file but restore the mtime recorded by the previous
    // reloadIfNeeded(): the reload must be skipped and the cached entries kept.
    try writeDictionary("でんわ\t☎️")
    try FileManager.default.setAttributes(
      [.modificationDate: wholeSecondMtime], ofItemAtPath: dictionaryPath.path)

    dictionary.reloadIfNeeded()
    XCTAssertEqual(dictionary.count, 1)
    XCTAssertTrue(dictionary.exactMatches(hiragana: "でんわ").isEmpty)
  }

  func testMissingFileClearsEntries() throws {
    try writeDictionary("めーる\tmail@example.com")
    let dictionary = UserDictionary()
    dictionary.reloadIfNeeded()
    XCTAssertEqual(dictionary.count, 1)

    try FileManager.default.removeItem(at: dictionaryPath)
    dictionary.reloadIfNeeded()
    XCTAssertEqual(dictionary.count, 0)
    XCTAssertTrue(dictionary.exactMatches(hiragana: "めーる").isEmpty)
  }
}

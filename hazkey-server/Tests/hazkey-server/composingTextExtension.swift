import Foundation
import XCTest

@testable import hazkeyServer

final class TextInputTests: BaseHazkeyServerTestCase {

  func testBasicHiraganaInput() throws {
    let inputQuery = QueryDataBuilder.inputText("あ")
    let inputResponse = try sendQuery(inputQuery)
    XCTAssertEqual(inputResponse.status, .success, "Hiragana input should succeed")

    let getStringQuery = QueryDataBuilder.getComposingString(charType: .hiragana)
    let stringResponse = try sendQuery(getStringQuery)
    XCTAssertEqual(stringResponse.status, .success)
    XCTAssertEqual(stringResponse.result, "あ", "Should return the input hiragana character")
  }

  func testMultipleCharacterInput() throws {
    let characters = ["あ", "い", "う"]

    for char in characters {
      let inputQuery = QueryDataBuilder.inputText(char)
      let response = try sendQuery(inputQuery)
      XCTAssertEqual(response.status, .success, "Input of '\(char)' should succeed")
    }

    let getStringQuery = QueryDataBuilder.getComposingString(charType: .hiragana)
    let stringResponse = try sendQuery(getStringQuery)
    XCTAssertEqual(stringResponse.status, .success)
    XCTAssertEqual(stringResponse.result, "あいう", "Should concatenate multiple hiragana characters")
  }

  func testDirectInput() throws {
    let inputQuery = QueryDataBuilder.inputText("A", isDirect: true)
    let inputResponse = try sendQuery(inputQuery)
    XCTAssertEqual(inputResponse.status, .success, "Direct input should succeed")

    let getStringQuery = QueryDataBuilder.getComposingString(charType: .hiragana)
    let stringResponse = try sendQuery(getStringQuery)
    XCTAssertEqual(stringResponse.status, .success)
    XCTAssertEqual(
      stringResponse.result, "A", "Direct input should preserve the original character")
  }

  func testEmptyStringInput() throws {
    let inputQuery = QueryDataBuilder.inputText("")
    let inputResponse = try sendQuery(inputQuery)
    // This should fail because empty string doesn't have a first unicode character
    XCTAssertEqual(inputResponse.status, .failed, "Empty string input should fail")
    XCTAssertFalse(
      inputResponse.errorMessage.isEmpty, "Should provide error message for empty input")
  }

  func testNumericInputWithFullwidthConfiguration() throws {
    // Set configuration for fullwidth numbers
    let configQuery = QueryDataBuilder.setConfig(numberFullwidth: 1)
    let configResponse = try sendQuery(configQuery)
    XCTAssertEqual(configResponse.status, .success)

    // Create new instance to apply config
    let instanceQuery = QueryDataBuilder.createComposingTextInstance()
    let instanceResponse = try sendQuery(instanceQuery)
    XCTAssertEqual(instanceResponse.status, .success)

    let inputQuery = QueryDataBuilder.inputText("123")
    let inputResponse = try sendQuery(inputQuery)
    XCTAssertEqual(inputResponse.status, .success)

    let getStringQuery = QueryDataBuilder.getComposingString(charType: .hiragana)
    let stringResponse = try sendQuery(getStringQuery)
    XCTAssertEqual(stringResponse.status, .success)
    XCTAssertEqual(stringResponse.result, "１", "Only first character should be processed")
  }

  func testCharacterTypeConversion() throws {
    let inputQuery = QueryDataBuilder.inputText("あ")
    let inputResponse = try sendQuery(inputQuery)
    XCTAssertEqual(inputResponse.status, .success)

    // Test different character type outputs
    let testCases: [(Hazkey_Commands_QueryData.GetComposingStringProps.CharType, String)] = [
      (.hiragana, "あ"),
      (.katakanaFull, "ア"),
      (.katakanaHalf, "ｱ"),
    ]

    for (charType, expected) in testCases {
      let getStringQuery = QueryDataBuilder.getComposingString(charType: charType)
      let stringResponse = try sendQuery(getStringQuery)
      XCTAssertEqual(stringResponse.status, .success)
      XCTAssertEqual(
        stringResponse.result, expected,
        "Character type \(charType) should return \(expected)")
    }
  }
}

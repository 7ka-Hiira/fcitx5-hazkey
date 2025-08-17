import Foundation
import XCTest

@testable import hazkeyServer

final class ErrorHandlingTests: BaseHazkeyServerTestCase {

  func testInvalidCharacterTypeInGetComposingString() throws {
    // First input some text
    let inputQuery = QueryDataBuilder.inputText("あ")
    let inputResponse = try sendQuery(inputQuery)
    XCTAssertEqual(inputResponse.status, .success)

    // Try to get composing string with invalid character type
    var query = Hazkey_Commands_QueryData()
    query.function = .getComposingString
    query.getComposingString = Hazkey_Commands_QueryData.GetComposingStringProps.with {
      $0.charType = .UNRECOGNIZED(999)  // Invalid char type
    }

    let response = try sendQuery(query)
    XCTAssertEqual(response.status, .failed, "Invalid character type should result in failure")
    XCTAssertFalse(
      response.errorMessage.isEmpty, "Should provide error message for invalid character type")
  }

  func testMultipleComposingTextInstanceCreation() throws {
    // Create first instance
    let firstInstanceQuery = QueryDataBuilder.createComposingTextInstance()
    let firstResponse = try sendQuery(firstInstanceQuery)
    XCTAssertEqual(firstResponse.status, .success)

    // Add some input
    let inputQuery = QueryDataBuilder.inputText("test")
    let inputResponse = try sendQuery(inputQuery)
    XCTAssertEqual(inputResponse.status, .success)

    // Create second instance (should reset the first one)
    let secondInstanceQuery = QueryDataBuilder.createComposingTextInstance()
    let secondResponse = try sendQuery(secondInstanceQuery)
    XCTAssertEqual(secondResponse.status, .success)

    // Check that composing text is reset
    let getStringQuery = QueryDataBuilder.getComposingString()
    let stringResponse = try sendQuery(getStringQuery)
    XCTAssertEqual(stringResponse.status, .success)
    XCTAssertEqual(stringResponse.result, "", "New instance should have empty composing text")
  }

  func testLargeInputString() throws {
    // Test with a very long string
    let largeString = String(repeating: "あ", count: 1000)

    // Note: The server only processes the first Unicode character
    let inputQuery = QueryDataBuilder.inputText(largeString)
    let inputResponse = try sendQuery(inputQuery)
    XCTAssertEqual(inputResponse.status, .success, "Large input should succeed")

    let getStringQuery = QueryDataBuilder.getComposingString()
    let stringResponse = try sendQuery(getStringQuery)
    XCTAssertEqual(stringResponse.status, .success)
    XCTAssertEqual(stringResponse.result, "あ", "Should only process first character")
  }
}

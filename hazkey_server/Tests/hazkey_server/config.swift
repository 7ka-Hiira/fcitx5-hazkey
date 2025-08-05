import Foundation
import XCTest

@testable import hazkey_server

final class ConfigurationTests: BaseHazkeyServerTestCase {
  func testSetCustomConfiguration() throws {
    let query = QueryDataBuilder.setConfig(
      commaStyle: 1,
      numberFullwidth: 1,
      periodStyle: 2,
      spaceFullwidth: 1,
      symbolFullwidth: 1,
      tenCombining: 1,
      zenzaiEnabled: true,
      zenzaiInferLimit: 5
    )

    let response = try sendQuery(query)

    XCTAssertEqual(
      response.status, .success,
      "Setting custom configuration should succeed")
    XCTAssertTrue(
      response.errorMessage.isEmpty,
      "Error message should be empty on success")
  }

  func testConfigurationPersistence() throws {
    // Set a custom configuration
    let customConfig = QueryDataBuilder.setConfig(
      numberFullwidth: 1,
      symbolFullwidth: 1
    )
    let configResponse = try sendQuery(customConfig)
    XCTAssertEqual(configResponse.status, .success)

    // Create new composing text instance to test persistence
    let instanceQuery = QueryDataBuilder.createComposingTextInstance()
    let instanceResponse = try sendQuery(instanceQuery)
    XCTAssertEqual(instanceResponse.status, .success)

    // Input number and check if it's converted to fullwidth
    let inputQuery = QueryDataBuilder.inputText("1")
    let inputResponse = try sendQuery(inputQuery)
    XCTAssertEqual(inputResponse.status, .success)

    let getStringQuery = QueryDataBuilder.getComposingString()
    let stringResponse = try sendQuery(getStringQuery)
    XCTAssertEqual(stringResponse.status, .success)

    // With fullwidth numbers enabled, "1" should become "１"
    XCTAssertEqual(
      stringResponse.result, "１",
      "Number should be converted to fullwidth when numberFullwidth is enabled")
  }
}

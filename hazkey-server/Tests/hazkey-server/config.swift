import Foundation
import XCTest

@testable import hazkey_server

final class ConfigurationTests: BaseHazkeyServerTestCase {
  func testSetCustomConfiguration() throws {
    let query = QueryDataBuilder.setConfig(
      numberFullwidth: true,
      symbolFullwidth: true,
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

    let currentConfigResponse = try sendQuery(QueryDataBuilder.getConfig())
    XCTAssertEqual(currentConfigResponse.status, .success)
    guard case .currentConfig(let currentConfig) = currentConfigResponse.payload,
      let profile = currentConfig.profiles.first
    else {
      return XCTFail("Response should contain the current profile")
    }
    XCTAssertTrue(profile.zenzaiEnable)
    XCTAssertEqual(profile.zenzaiInferLimit, 5)
  }

  func testConfigurationPersistence() throws {
    // Set a custom configuration
    let customConfig = QueryDataBuilder.setConfig(
      numberFullwidth: true,
      symbolFullwidth: true
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
      stringResponse.text, "１",
      "Number should be converted to fullwidth when numberFullwidth is enabled")
  }
}

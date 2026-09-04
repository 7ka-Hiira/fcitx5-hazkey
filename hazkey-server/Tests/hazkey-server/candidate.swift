import Foundation
import XCTest

@testable import hazkey_server

final class CandidateTests: BaseHazkeyServerTestCase {

  func testGetCandidatesWithEmptyInput() throws {
    let candidatesQuery = QueryDataBuilder.getCandidates()
    let candidatesResponse = try sendQuery(candidatesQuery)

    XCTAssertEqual(
      candidatesResponse.status, .success, "Getting candidates should succeed even with empty input"
    )

    if case .candidates(let candidatesResult) = candidatesResponse.payload {
      XCTAssertTrue(candidatesResult.candidates.isEmpty)
    } else {
      XCTFail("Response should contain candidates")
    }
  }

  func testGetCandidatesWithHiraganaInput() throws {
    // Input some hiragana
    let inputQuery = QueryDataBuilder.inputText("あい")
    let inputResponse = try sendQuery(inputQuery)
    XCTAssertEqual(inputResponse.status, .success)

    let candidatesQuery = QueryDataBuilder.getCandidates()
    let candidatesResponse = try sendQuery(candidatesQuery)

    XCTAssertEqual(candidatesResponse.status, .success, "Getting candidates should succeed")

    if case .candidates(let candidatesResult) = candidatesResponse.payload {
      XCTAssertFalse(
        candidatesResult.candidates.isEmpty, "Should return some candidates for hiragana input")

      // Check first candidate structure
      if let firstCandidate = candidatesResult.candidates.first {
        XCTAssertFalse(firstCandidate.text.isEmpty, "Candidate text should not be empty")
      }
    } else {
      XCTFail("Response should contain candidates")
    }
  }

  func testGetCandidatesUsesConfiguredPageSize() throws {
    let configResponse = try sendQuery(QueryDataBuilder.setConfig())
    XCTAssertEqual(configResponse.status, .success)

    let inputQuery = QueryDataBuilder.inputText("あ")
    let inputResponse = try sendQuery(inputQuery)
    XCTAssertEqual(inputResponse.status, .success)

    let candidatesQuery = QueryDataBuilder.getCandidates()
    let candidatesResponse = try sendQuery(candidatesQuery)

    XCTAssertEqual(candidatesResponse.status, .success)

    if case .candidates(let candidatesResult) = candidatesResponse.payload {
      XCTAssertEqual(candidatesResult.pageSize, 9)
    } else {
      XCTFail("Response should contain candidates")
    }
  }

  func testGetCandidatesInPredictMode() throws {
    let inputQuery = QueryDataBuilder.inputText("こん")
    let inputResponse = try sendQuery(inputQuery)
    XCTAssertEqual(inputResponse.status, .success)

    let candidatesQuery = QueryDataBuilder.getCandidates(isSuggest: true)
    let candidatesResponse = try sendQuery(candidatesQuery)

    XCTAssertEqual(candidatesResponse.status, .success, "Predict mode should work")

    if case .candidates(let candidatesResult) = candidatesResponse.payload {
      XCTAssertFalse(candidatesResult.candidates.isEmpty)
    } else {
      XCTFail("Response should contain candidates")
    }
  }
}

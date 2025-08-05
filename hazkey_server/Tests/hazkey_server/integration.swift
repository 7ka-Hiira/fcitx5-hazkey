import Foundation
import XCTest
@testable import hazkey_server

final class IntegrationTests: BaseHazkeyServerTestCase {
    
    func testCompleteInputWorkflow() throws {
        // 1. Set custom configuration
        let configQuery = QueryDataBuilder.setConfig(
            numberFullwidth: 1,
            symbolFullwidth: 1
        )
        let configResponse = try sendQuery(configQuery)
        XCTAssertEqual(configResponse.status, .success)
        
        // 2. Create composing text instance
        let instanceQuery = QueryDataBuilder.createComposingTextInstance()
        let instanceResponse = try sendQuery(instanceQuery)
        XCTAssertEqual(instanceResponse.status, .success)
        
        // 3. Input multiple characters
        let inputChars = ["こ", "ん", "に", "ち", "は"]
        for char in inputChars {
            let inputQuery = QueryDataBuilder.inputText(char)
            let inputResponse = try sendQuery(inputQuery)
            XCTAssertEqual(inputResponse.status, .success, "Input of '\(char)' should succeed")
        }
        
        // 4. Get composing string
        let getStringQuery = QueryDataBuilder.getComposingString(charType: .hiragana)
        let stringResponse = try sendQuery(getStringQuery)
        XCTAssertEqual(stringResponse.status, .success)
        XCTAssertEqual(stringResponse.result, "こんにちは", "Should compose complete hiragana string")
        
        // 5. Get candidates
        let candidatesQuery = QueryDataBuilder.getCandidates()
        let candidatesResponse = try sendQuery(candidatesQuery)
        XCTAssertEqual(candidatesResponse.status, .success)
        
        if case .candidates(let candidatesResult) = candidatesResponse.props {
            XCTAssertFalse(candidatesResult.candidates.isEmpty, "Should return candidates for 'こんにちは'")
            
            // Check if we get "こんにちは" or "今日は" as candidates
            let candidateTexts = candidatesResult.candidates.map { $0.text }
            XCTAssertTrue(candidateTexts.contains("こんにちは") || candidateTexts.contains("今日は"),
                         "Should contain greeting candidates")
        } else {
            XCTFail("Should receive candidates")
        }
    }
    
    func testNumberAndSymbolConversion() throws {
        // Configure for fullwidth conversion
        let configQuery = QueryDataBuilder.setConfig(
            numberFullwidth: 1,
            symbolFullwidth: 1
        )
        let configResponse = try sendQuery(configQuery)
        XCTAssertEqual(configResponse.status, .success)
        
        let instanceQuery = QueryDataBuilder.createComposingTextInstance()
        let instanceResponse = try sendQuery(instanceQuery)
        XCTAssertEqual(instanceResponse.status, .success)
        
        // Test number conversion
        let numberInputQuery = QueryDataBuilder.inputText("5")
        let numberResponse = try sendQuery(numberInputQuery)
        XCTAssertEqual(numberResponse.status, .success)
        
        let getNumberQuery = QueryDataBuilder.getComposingString()
        let numberStringResponse = try sendQuery(getNumberQuery)
        XCTAssertEqual(numberStringResponse.status, .success)
        XCTAssertEqual(numberStringResponse.result, "５", "Number should be converted to fullwidth")
    }
    
    func testMultipleSessionsSequentially() throws {
        // Session 1
        let session1InstanceQuery = QueryDataBuilder.createComposingTextInstance()
        let session1Response = try sendQuery(session1InstanceQuery)
        XCTAssertEqual(session1Response.status, .success)
        
        let session1InputQuery = QueryDataBuilder.inputText("あ")
        let session1InputResponse = try sendQuery(session1InputQuery)
        XCTAssertEqual(session1InputResponse.status, .success)
        
        // Session 2 (new instance)
        let session2InstanceQuery = QueryDataBuilder.createComposingTextInstance()
        let session2Response = try sendQuery(session2InstanceQuery)
        XCTAssertEqual(session2Response.status, .success)
        
        // Session 2 should have clean state
        let session2GetQuery = QueryDataBuilder.getComposingString()
        let session2StringResponse = try sendQuery(session2GetQuery)
        XCTAssertEqual(session2StringResponse.status, .success)
        XCTAssertEqual(session2StringResponse.result, "", "New session should start with empty state")
    }
}

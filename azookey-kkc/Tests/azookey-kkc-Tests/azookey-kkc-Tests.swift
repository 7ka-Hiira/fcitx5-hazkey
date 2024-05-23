import XCTest
@testable import azookey_kkc

final class fcitx5_azookeyTests: XCTestCase {
    func testAscii2Hiragana() {
        let ascii = "azu-ki-"
        let expectedHiragana = "あずーきー"
        print("ascii: \(ascii)")
        
        let kana = ascii2hiragana(ascii: ascii)
        
        XCTAssertEqual(kana, expectedHiragana)
    }
}

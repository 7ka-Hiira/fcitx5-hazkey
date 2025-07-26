import Foundation
import KanaKanjiConverterModule
import SwiftUtils
import XCTest

@testable import hazkey_server

final class ComposingTextExtensionTests: XCTestCase {
  static let copmosingTextTestSources: [String] = [
    "konnnichiha",
    "yaxXxxxxxtsutaー",
    "kansha",
    "あがょぱ＝＃",
  ]

  func testToHiragana() {
    let expecteds = [
      "こんにちは",
      "やxXっっっっったー",
      "かんしゃ",
      "あがょぱ＝＃",
    ]
    for (i, source) in Self.copmosingTextTestSources.enumerated() {
      var composingText = ComposingText()
      composingText.insertAtCursorPosition(source, inputStyle: .roman2kana)
      XCTAssertEqual(composingText.toHiragana(), expecteds[i])
    }
  }

  func testToKatakana() {
    let expecteds_full = [
      "コンニチハ",
      "ヤxXッッッッッター",
      "カンシャ",
      "アガョパ＝＃",
    ]
    let expecteds_half = [
      "ｺﾝﾆﾁﾊ",
      "ﾔxXｯｯｯｯｯﾀｰ",
      "ｶﾝｼｬ",
      "ｱｶﾞｮﾊﾟ=#",
    ]
    for (i, source) in Self.copmosingTextTestSources.enumerated() {
      var composingText = ComposingText()
      composingText.insertAtCursorPosition(source, inputStyle: .roman2kana)
      XCTAssertEqual(composingText.toKatakana(true), expecteds_full[i])
      XCTAssertEqual(composingText.toKatakana(false), expecteds_half[i])
    }
  }

  func testToAlphabet() {
    let expecteds_full = [
      "ｋｏｎｎｎｉｃｈｉｈａ",
      "ｙａｘＸｘｘｘｘｘｔｓｕｔａ－",
      "ｋａｎｓｈａ",
      "あがょぱ＝＃",
    ]
    let expecteds_half = [
      "konnnichiha",
      "yaxXxxxxxtsutaｰ",
      "kansha",
      "あがょぱ=#",
    ]
    for (i, source) in Self.copmosingTextTestSources.enumerated() {
      var composingText = ComposingText()
      composingText.insertAtCursorPosition(source, inputStyle: .roman2kana)
      XCTAssertEqual(composingText.toAlphabet(true), expecteds_full[i])
      XCTAssertEqual(composingText.toAlphabet(false), expecteds_half[i])
    }
  }
}

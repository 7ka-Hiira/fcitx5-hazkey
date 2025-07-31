import Foundation
import XCTest

@testable import hazkey_server

final class ProtocolTests: XCTestCase {

  func socketPath() -> String {
    let runtimeDir = ProcessInfo.processInfo.environment["XDG_RUNTIME_DIR"] ?? "/tmp"
    let uid = getuid()
    return "\(runtimeDir)/hazkey_server.\(uid).sock"
  }

  func waitForSocket(timeout: TimeInterval = 2.0) -> Bool {
    let path = socketPath()
    let start = Date()
    while !FileManager.default.fileExists(atPath: path) {
      usleep(50_000)
      if Date().timeIntervalSince(start) > timeout {
        return false
      }
    }
    return true
  }

  func connectToServer() throws -> Int32 {
    let path = socketPath()
    let clientSocket = socket(AF_UNIX, Int32(SOCK_STREAM.rawValue), 0)
    var addr = sockaddr_un()
    addr.sun_family = sa_family_t(AF_UNIX)
    withUnsafeMutablePointer(to: &addr.sun_path) {
      $0.withMemoryRebound(to: UInt8.self, capacity: 108) { ptr in
        let pathData = path.data(using: .utf8)!
        for i in 0..<min(pathData.count, 108) {
          ptr[i] = pathData[i]
        }
      }
    }
    let addrSize = MemoryLayout.size(ofValue: addr)
    let connectResult = withUnsafePointer(to: &addr) {
      $0.withMemoryRebound(to: sockaddr.self, capacity: 1) {
        connect(clientSocket, $0, socklen_t(addrSize))
      }
    }
    guard connectResult == 0 else {
      close(clientSocket)
      throw NSError(domain: "ConnectFailed", code: Int(connectResult), userInfo: nil)
    }
    return clientSocket
  }

  func createComposingTextInstance(socket: Int32) throws {
    var query = Hazkey_Commands_QueryData()
    query.function = Hazkey_Commands_QueryData.KkcApi.createComposingTextInstance
    let reqData = try query.serializedData()
    let responseData = try sendRequest(reqData, clientSocket: socket)
    if let responseMsg = try? Hazkey_Commands_ResultData(serializedBytes: responseData) {
      XCTAssertEqual(Hazkey_Commands_ResultData.StatusCode.success, responseMsg.status)
    } else {
      XCTFail("Failed to decode response1")
    }
  }

  func sendRequest(_ reqData: Data, clientSocket: Int32) throws -> Data {
    _ = reqData.withUnsafeBytes { write(clientSocket, $0.baseAddress, reqData.count) }
    var buffer = [UInt8](repeating: 0, count: 65536)
    let bytesRead = read(clientSocket, &buffer, buffer.count)
    guard bytesRead > 0 else {
      throw NSError(domain: "ReadFailed", code: 0, userInfo: nil)
    }
    return Data(buffer[0..<bytesRead])
  }

  func testUnixSocketProtobufRequestResponse() throws {
    XCTAssertTrue(waitForSocket(), "Server socket did not appear in time")
    let clientSocket = try connectToServer()
    defer { close(clientSocket) }

    try createComposingTextInstance(socket: clientSocket)

    var query = Hazkey_Commands_QueryData()
    query.function = .getComposingString
    query.getComposingString = Hazkey_Commands_QueryData.GetComposingStringProps.with {
      $0.charType = .hiragana
    }
    let reqData = try query.serializedData()
    let responseData = try sendRequest(reqData, clientSocket: clientSocket)

    if let responseMsg = try? Hazkey_Commands_ResultData(serializedBytes: responseData) {
      XCTAssertEqual(Hazkey_Commands_ResultData.StatusCode.success, responseMsg.status)
      XCTAssertEqual("", responseMsg.result)
    } else {
      XCTFail("Failed to decode response")
    }
  }

  func testInputRequest() throws {
    XCTAssertTrue(waitForSocket(), "Server socket did not appear in time")
    let clientSocket = try connectToServer()
    defer { close(clientSocket) }

    var query0 = Hazkey_Commands_QueryData()
    query0.function = Hazkey_Commands_QueryData.KkcApi.setConfig
    query0.setConfig = Hazkey_Commands_QueryData.SetConfigProps.with {
      $0.commaStyle = 0
      $0.numberFullwidth = 0
      $0.periodStyle = 0
      $0.profileText = ""
      $0.spaceFullwidth = 0
      $0.symbolFullwidth = 0
      $0.tenCombining = 0
      $0.zenzaiEnabled = false
      $0.zenzaiInferLimit = 1
    }
    let reqData0 = try query0.serializedData()
    let responseData0 = try sendRequest(reqData0, clientSocket: clientSocket)
    if let responseMsg = try? Hazkey_Commands_ResultData(serializedBytes: responseData0) {
      XCTAssertEqual(Hazkey_Commands_ResultData.StatusCode.success, responseMsg.status)
    } else {
      XCTFail("Failed to decode response0")
    }

    try createComposingTextInstance(socket: clientSocket)

    var query2 = Hazkey_Commands_QueryData()
    query2.function = Hazkey_Commands_QueryData.KkcApi.inputText
    query2.inputText = Hazkey_Commands_QueryData.InputTextProps.with {
      $0.text = "3"
      $0.isDirect = false
    }
    let reqData2 = try query2.serializedData()
    let responseData2 = try sendRequest(reqData2, clientSocket: clientSocket)
    if let responseMsg = try? Hazkey_Commands_ResultData(serializedBytes: responseData2) {
      XCTAssertEqual(Hazkey_Commands_ResultData.StatusCode.success, responseMsg.status)
    } else {
      XCTFail("Failed to decode response2")
    }

    var query3 = Hazkey_Commands_QueryData()
    query3.function = Hazkey_Commands_QueryData.KkcApi.getComposingString
    query3.getComposingString = Hazkey_Commands_QueryData.GetComposingStringProps.with {
      $0.charType = Hazkey_Commands_QueryData.GetComposingStringProps.CharType.hiragana
    }
    let reqData3 = try query3.serializedData()
    let responseData3 = try sendRequest(reqData3, clientSocket: clientSocket)
    if let responseMsg3 = try? Hazkey_Commands_ResultData(serializedBytes: responseData3) {
      XCTAssertEqual(Hazkey_Commands_ResultData.StatusCode.success, responseMsg3.status)
      XCTAssertEqual("3", responseMsg3.result)
    } else {
      XCTFail("Failed to decode response3")
    }
  }
}

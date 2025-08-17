import Foundation
import SwiftGlibc
import XCTest

@testable import hazkeyServer

// MARK: - Test Configuration
struct TestConfig {
  static let defaultTimeout: TimeInterval = 5.0
  static let socketCheckInterval: useconds_t = 50_000  // 50ms
  static let maxRetries = 3
}

// MARK: - Test Data Builders
struct QueryDataBuilder {
  static func setConfig(
    commaStyle: Int32 = 0,
    numberFullwidth: Int32 = 0,
    periodStyle: Int32 = 0,
    spaceFullwidth: Int32 = 0,
    symbolFullwidth: Int32 = 0,
    tenCombining: Int32 = 0,
    zenzaiEnabled: Bool = false,
    zenzaiInferLimit: Int32 = 1
  ) -> Hazkey_Commands_QueryData {
    var query = Hazkey_Commands_QueryData()
    query.function = .setConfig
    query.setConfig = Hazkey_Commands_QueryData.SetConfigProps.with {
      $0.commaStyle = commaStyle
      $0.numberFullwidth = numberFullwidth
      $0.periodStyle = periodStyle
      $0.profileText = ""
      $0.spaceFullwidth = spaceFullwidth
      $0.symbolFullwidth = symbolFullwidth
      $0.tenCombining = tenCombining
      $0.zenzaiEnabled = zenzaiEnabled
      $0.zenzaiInferLimit = zenzaiInferLimit
    }
    return query
  }

  static func inputText(_ text: String, isDirect: Bool = false) -> Hazkey_Commands_QueryData {
    var query = Hazkey_Commands_QueryData()
    query.function = .inputText
    query.inputText = Hazkey_Commands_QueryData.InputTextProps.with {
      $0.text = text
      $0.isDirect = isDirect
    }
    return query
  }

  static func getComposingString(
    charType: Hazkey_Commands_QueryData.GetComposingStringProps.CharType = .hiragana
  ) -> Hazkey_Commands_QueryData {
    var query = Hazkey_Commands_QueryData()
    query.function = .getComposingString
    query.getComposingString = Hazkey_Commands_QueryData.GetComposingStringProps.with {
      $0.charType = charType
    }
    return query
  }

  static func createComposingTextInstance() -> Hazkey_Commands_QueryData {
    var query = Hazkey_Commands_QueryData()
    query.function = .createComposingTextInstance
    return query
  }

  static func getCandidates(
    nBest: Int32 = 9,
    isPredictMode: Bool = false
  ) -> Hazkey_Commands_QueryData {
    var query = Hazkey_Commands_QueryData()
    query.function = .getCandidates
    query.getCandidates = Hazkey_Commands_QueryData.GetCandidatesProps.with {
      $0.nBest = nBest
      $0.isPredictMode = isPredictMode
    }
    return query
  }
}

// MARK: - Server Client
class HazkeyServerClient {
  private var socket: Int32?
  private let socketPath: String

  init() {
    let runtimeDir = ProcessInfo.processInfo.environment["XDG_RUNTIME_DIR"] ?? "/tmp"
    let uid = getuid()
    self.socketPath = "\(runtimeDir)/hazkey-server.\(uid).sock"
  }

  func connect() throws {
    guard socket == nil else { return }  // Already connected

    let clientSocket = SwiftGlibc.socket(AF_UNIX, Int32(SOCK_STREAM.rawValue), 0)
    guard clientSocket != -1 else {
      throw TestError.socketCreationFailed
    }

    var addr = sockaddr_un()
    addr.sun_family = sa_family_t(AF_UNIX)

    try withUnsafeMutablePointer(to: &addr.sun_path) {
      try $0.withMemoryRebound(to: UInt8.self, capacity: 108) { ptr in
        guard let pathData = socketPath.data(using: .utf8) else {
          throw TestError.pathDataNotFound
        }
        guard pathData.count < 108 else {
          throw TestError.pathTooLong
        }
        for i in 0..<pathData.count {
          ptr[i] = pathData[i]
        }
      }
    }

    let addrSize = MemoryLayout.size(ofValue: addr)
    let connectResult = withUnsafePointer(to: &addr) {
      $0.withMemoryRebound(to: sockaddr.self, capacity: 1) {
        SwiftGlibc.connect(clientSocket, $0, socklen_t(addrSize))
      }
    }

    guard connectResult == 0 else {
      close(clientSocket)
      throw TestError.connectionFailed(errno: connectResult)
    }

    self.socket = clientSocket
  }

  func disconnect() {
    if let socket = socket {
      close(socket)
      self.socket = nil
    }
  }

  func sendQuery(_ query: Hazkey_Commands_QueryData) throws -> Hazkey_Commands_ResultData {
    guard let socket = socket else {
      throw TestError.notConnected
    }

    let reqData = try query.serializedData()
    let responseData = try sendRequest(reqData, socket: socket)

    return try Hazkey_Commands_ResultData(serializedBytes: responseData)
  }

  private func sendRequest(_ reqData: Data, socket: Int32) throws -> Data {
    // Send request size
    var size = Int32(reqData.count).bigEndian
    let sizeData = Data(bytes: &size, count: MemoryLayout<Int32>.size)

    try write(data: sizeData, to: socket)
    try write(data: reqData, to: socket)

    // Read response size
    let responseSizeData = try read(bytes: 4, from: socket)
    let responseSize = Int(
      Int32(bigEndian: responseSizeData.withUnsafeBytes { $0.load(as: Int32.self) }))

    // Read response data
    return try read(bytes: responseSize, from: socket)
  }

  private func write(data: Data, to socket: Int32) throws {
    var bytesWritten = 0
    while bytesWritten < data.count {
      let result = data.withUnsafeBytes { bytes in
        SwiftGlibc.write(
          socket, bytes.baseAddress!.advanced(by: bytesWritten), data.count - bytesWritten)
      }

      guard result > 0 else {
        throw TestError.writeFailed(errno: errno)
      }

      bytesWritten += result
    }
  }

  private func read(bytes count: Int, from socket: Int32) throws -> Data {
    var buffer = [UInt8](repeating: 0, count: count)
    var totalRead = 0

    while totalRead < count {
      let bytesRead = SwiftGlibc.read(socket, &buffer[totalRead], count - totalRead)
      guard bytesRead > 0 else {
        throw TestError.readFailed(errno: errno)
      }
      totalRead += bytesRead
    }

    return Data(buffer)
  }
}

// MARK: - Test Errors
enum TestError: Error, LocalizedError {
  case socketCreationFailed
  case pathDataNotFound
  case pathTooLong
  case connectionFailed(errno: Int32)
  case notConnected
  case writeFailed(errno: Int32)
  case readFailed(errno: Int32)
  case serverNotReady
  case unexpectedResponse

  var errorDescription: String? {
    switch self {
    case .socketCreationFailed:
      return "Failed to create socket"
    case .pathDataNotFound:
      return "Cannot found path data"
    case .pathTooLong:
      return "Socket path is too long"
    case .connectionFailed(let errno):
      return "Connection failed with errno: \(errno)"
    case .notConnected:
      return "Client is not connected to server"
    case .writeFailed(let errno):
      return "Write failed with errno: \(errno)"
    case .readFailed(let errno):
      return "Read failed with errno: \(errno)"
    case .serverNotReady:
      return "Server is not ready"
    case .unexpectedResponse:
      return "Received unexpected response from server"
    }
  }
}

// MARK: - Test Utilities
class TestUtilities {
  static func waitForServer(timeout: TimeInterval = TestConfig.defaultTimeout) -> Bool {
    let runtimeDir = ProcessInfo.processInfo.environment["XDG_RUNTIME_DIR"] ?? "/tmp"
    let uid = getuid()
    let socketPath = "\(runtimeDir)/hazkey-server.\(uid).sock"

    let start = Date()
    while !FileManager.default.fileExists(atPath: socketPath) {
      usleep(TestConfig.socketCheckInterval)
      if Date().timeIntervalSince(start) > timeout {
        return false
      }
    }
    return true
  }

  static func retryOperation<T>(
    _ operation: () throws -> T, maxRetries: Int = TestConfig.maxRetries
  ) throws -> T {
    var lastError: Error?

    for attempt in 0...maxRetries {
      do {
        return try operation()
      } catch {
        lastError = error
        if attempt < maxRetries {
          usleep(100_000)  // Wait 100ms before retry
        }
      }
    }

    throw lastError ?? TestError.unexpectedResponse
  }
}

import Foundation
import XCTest

@testable import hazkey_server

final class ProtocolTests: XCTestCase {
    func testUnixSocketProtobufCommunication() throws {
        let socketPath = "/tmp/test.sock"
        let messageText = "Hello, Protobuf!"
        let server = UnixSocketServer(socketPath: socketPath)
        let serverExpectation = expectation(description: "Server received message")

        server.start { data in
            if let receivedMsg = try? TestMessage(serializedBytes: data) {
                XCTAssertEqual(receivedMsg.msg, messageText)
                serverExpectation.fulfill()
            }
        }

        let timeout: TimeInterval = 2.0
        let start = Date()
        while !FileManager.default.fileExists(atPath: socketPath) {
            usleep(50_000)
            if Date().timeIntervalSince(start) > timeout {
                XCTFail("Server socket did not appear in time")
                return
            }
        }

        let clientSocket = socket(AF_UNIX, Int32(SOCK_STREAM.rawValue), 0)
        var addr = sockaddr_un()
        addr.sun_family = sa_family_t(AF_UNIX)
        withUnsafeMutablePointer(to: &addr.sun_path) {
            $0.withMemoryRebound(to: UInt8.self, capacity: 108) { ptr in
                let pathData = socketPath.data(using: .utf8)!
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
        XCTAssertEqual(connectResult, 0)

        let testMsg = TestMessage.with { $0.msg = messageText }
        let data = try testMsg.serializedData()
        _ = data.withUnsafeBytes { write(clientSocket, $0.baseAddress, data.count) }
        close(clientSocket)

        wait(for: [serverExpectation], timeout: 2.0)
        server.stop()
    }
}

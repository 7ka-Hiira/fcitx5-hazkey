import Foundation

class UnixSocketServer: @unchecked Sendable {
  let socketPath: String
  private var serverSocket: Int32 = -1
  private var running = false

  init(socketPath: String) {
    self.socketPath = socketPath
  }

  func start(handler: @escaping @Sendable (Data) -> Void) {
    DispatchQueue.global().async {
      self.running = true
      self.serverSocket = socket(AF_UNIX, Int32(SOCK_STREAM.rawValue), 0)
      var addr = sockaddr_un()
      addr.sun_family = sa_family_t(AF_UNIX)
      withUnsafeMutablePointer(to: &addr.sun_path) {
        $0.withMemoryRebound(to: UInt8.self, capacity: 108) { ptr in
          let pathData = self.socketPath.data(using: .utf8)!
          for i in 0..<min(pathData.count, 108) {
            ptr[i] = pathData[i]
          }
        }
      }
      unlink(self.socketPath)

      let addrSize = MemoryLayout.size(ofValue: addr)
      let bindResult = withUnsafePointer(to: &addr) {
        $0.withMemoryRebound(to: sockaddr.self, capacity: 1) {
          bind(self.serverSocket, $0, socklen_t(addrSize))
        }
      }
      guard bindResult == 0 else { return }
      listen(self.serverSocket, 1)
      while self.running {
        let clientSocket = accept(self.serverSocket, nil, nil)
        if clientSocket >= 0 {
          var buf = [UInt8](repeating: 0, count: 1024)
          let len = read(clientSocket, &buf, 1024)
          if len > 0 {
            handler(Data(buf[0..<len]))
          }
          close(clientSocket)
        }
      }
      close(self.serverSocket)
      unlink(self.socketPath)
    }
  }

  func stop() {
    running = false
    if serverSocket >= 0 { close(serverSocket) }
    unlink(socketPath)
  }
}

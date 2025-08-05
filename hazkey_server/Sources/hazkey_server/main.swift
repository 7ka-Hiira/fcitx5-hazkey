import Dispatch
import Foundation
import KanaKanjiConverterModule

let runtimeDir = ProcessInfo.processInfo.environment["XDG_RUNTIME_DIR"] ?? "/tmp"
let uid = getuid()
let socketPath = "\(runtimeDir)/hazkey_server.\(uid).sock"
let pidFilePath = "\(runtimeDir)/hazkey_server.\(uid).pid"

func removePidFile() {
  try? FileManager.default.removeItem(atPath: pidFilePath)
}

signal(SIGTERM) { _ in
  removePidFile()
  exit(0)
}
signal(SIGINT) { _ in
  removePidFile()
  exit(0)
}
signal(SIGPIPE) { _ in
  // Ignore SIGPIPE
  print("SIGPIPE received - client disconnected")
}

if FileManager.default.fileExists(atPath: pidFilePath) {
  if let pidString = try? String(contentsOfFile: pidFilePath, encoding: .utf8),
    let pid = pid_t(pidString)
  {
    if kill(pid, 0) == 0 {
      print("Another hazkey_server is already running.")
      exit(0)
    }
  }
  try? FileManager.default.removeItem(atPath: pidFilePath)
}
try? "\(getpid())".write(toFile: pidFilePath, atomically: true, encoding: .utf8)

unlink(socketPath)

let fd = socket(AF_UNIX, Int32(SOCK_STREAM.rawValue), 0)
guard fd != -1 else {
  fatalError("Failed to create socket")
}

var addr = sockaddr_un()
addr.sun_family = sa_family_t(AF_UNIX)
strncpy(&addr.sun_path.0, socketPath, MemoryLayout.size(ofValue: addr.sun_path))

let bindResult = withUnsafePointer(to: &addr) {
  $0.withMemoryRebound(to: sockaddr.self, capacity: 1) {
    bind(fd, $0, socklen_t(MemoryLayout.size(ofValue: addr)))
  }
}

guard bindResult != -1 else {
  fatalError("Failed to bind socket")
}

guard chmod(socketPath, 0o600) != -1 else {
  fatalError("Failed to set socket permissions")
}

guard listen(fd, 10) != -1 else {
  fatalError("Failed to listen")
}

// FIXME: unglobalize these vars
var kkcConfig: KkcConfig? = nil
var composingText: ComposingTextBox = ComposingTextBox()
var currentCandidateList: [Candidate]? = nil
var currentPreedit: String = ""

// FIXME: read config file
let _ = setConfig(
  zenzaiEnabled: false, zenzaiInferLimit: 1, numberFullwidth: 0, symbolFullwidth: 0,
  periodStyleIndex: 0, commaStyleIndex: 0, spaceFullwidth: 0, tenCombining: 0, profileText: "")
// set non-blocking
var flags = fcntl(fd, F_GETFL, 0)
let fcntlRes = fcntl(fd, F_SETFL, flags | O_NONBLOCK)
if fcntlRes != 0 {
  print("fcntl() failed")
}

print("start listening...")

var currentClientFd: Int32? = nil

while true {
  var pollFds: [pollfd] = []

  // Always poll the server socket for new connections
  pollFds.append(pollfd(fd: fd, events: Int16(POLLIN), revents: 0))

  // If we have a current client, also poll it
  if let clientFd = currentClientFd {
    pollFds.append(pollfd(fd: clientFd, events: Int16(POLLIN), revents: 0))
  }

  let pollRes = poll(&pollFds, nfds_t(pollFds.count), 1000)  // 1 second timeout

  if pollRes < 0 {
    print("Poll failed: \(errno)")
    break
  }

  if pollRes == 0 {
    // Timeout - continue polling
    continue
  }

  // Check if server socket has a new connection
  if pollFds[0].revents & Int16(POLLIN) != 0 {
    var clientAddr = sockaddr()
    var clientLen: socklen_t = socklen_t(MemoryLayout<sockaddr>.size)
    let newClientFd = accept(fd, &clientAddr, &clientLen)

    if newClientFd != -1 {
      // If we already have a client, close it
      if let existingClientFd = currentClientFd {
        print("New client connecting, closing existing client: \(existingClientFd)")
        close(existingClientFd)
      }

      // Set up the new client
      print("Client connected: \(newClientFd)")

      // Make client non-blocking
      let clientFlags = fcntl(newClientFd, F_GETFL, 0)
      let fcntlRes = fcntl(newClientFd, F_SETFL, clientFlags | O_NONBLOCK)
      if fcntlRes != 0 {
        print("fcntl() failed for client")
        close(newClientFd)
        currentClientFd = nil
      } else {
        currentClientFd = newClientFd
      }
    }
  }

  // Check if current client has data
  if let clientFd = currentClientFd, pollFds.count > 1 {
    let clientEvents = Int32(pollFds[1].revents)

    if clientEvents & POLLHUP != 0 || clientEvents & POLLERR != 0 {
      print("Client disconnected or error: \(clientFd)")
      close(clientFd)
      currentClientFd = nil
      continue
    }

    if clientEvents & POLLIN != 0 {
      do {
        // Handle client request
        let maxMessageSize: UInt32 = 1024 * 1024  // 1MB limit

        // Read message length header (4 bytes, big-endian)
        print("Reading data from client \(clientFd)...")
        let lengthData = try readData(from: clientFd, count: 4)
        let readLen = lengthData.withUnsafeBytes { $0.load(as: UInt32.self).bigEndian }
        print("Message length: \(readLen)")

        // Sanity check
        guard readLen <= maxMessageSize else {
          throw SocketError.messageTooLarge(readLen)
        }

        // Read message body
        let query = try readData(from: clientFd, count: Int(readLen))
        print("Successfully read \(query.count) bytes")

        // Process and respond
        let response = processProto(data: query)
        print("Processed request, response size: \(response.count)")

        // Write response length
        var writeLen = UInt32(response.count).bigEndian
        let lengthHeader = withUnsafeBytes(of: &writeLen) { Data($0) }
        try writeData(to: clientFd, data: lengthHeader)

        // Write response body
        try writeData(to: clientFd, data: response)

        fsync(clientFd)
        print("Successfully wrote response")

      } catch let error as SocketError {
        switch error {
        case .clientDisconnected(let msg):
          print(msg)
        case .readFailed(let msg, let err):
          print("Read failed: \(msg), errno: \(err)")
        case .incompleteRead(let msg), .incompleteWrite(let msg):
          print(msg)
        case .messageTooLarge(let len):
          print("Message too large: \(len)")
        case .writeFailed(let msg, let err):
          print("Write failed: \(msg), errno: \(err)")
        default:
          print("Socket error: \(error)")
        }
        print("Closing client connection due to error: \(clientFd)")
        close(clientFd)
        currentClientFd = nil
      } catch {
        print("An unexpected error occurred: \(error)")
        print("Closing client connection: \(clientFd)")
        close(clientFd)
        currentClientFd = nil
      }
    }
  }
}

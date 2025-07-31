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

if FileManager.default.fileExists(atPath: pidFilePath) {
  if let pidString = try? String(contentsOfFile: pidFilePath, encoding: .utf8),
    let pid = Int32(pidString.trimmingCharacters(in: .whitespacesAndNewlines))
  {
    if kill(pid, 0) != 0 {
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

// TODO: unglobalize these vars
var kkcConfig: KkcConfig? = nil
var composingText: ComposingTextBox = ComposingTextBox()
var currentCandidateList: [Candidate]? = nil
var currentPreedit: String = ""

var currentClientFd: Int32? = nil

while true {
  var clientAddr = sockaddr()
  var clientLen: socklen_t = socklen_t(MemoryLayout<sockaddr>.size)
  let clientFd = accept(fd, &clientAddr, &clientLen)
  if clientFd == -1 {
    print("{\"status\": \"error\", \"message\": \"Accept failed\"}")
    continue
  }

  if let oldFd = currentClientFd {
    close(oldFd)
  }
  currentClientFd = clientFd

  while true {
    // read
    var readLenBuf = [UInt8](repeating: 0, count: 4)
    _ = read(clientFd, &readLenBuf, 4)
    let readLen = UInt32(bigEndian: readLenBuf.withUnsafeBytes { $0.load(as: UInt32.self) })
    var buf = [UInt8](repeating: 0, count: Int(readLen))
    let readBytes = read(clientFd, &buf, buf.count)
    if readBytes <= 0 { break }
    let query = Data(buf[0..<readBytes])
    // write
    let response = processProto(data: query)
    var writeLen = UInt32(response.count).bigEndian
    _ = withUnsafeBytes(of: &writeLen) { write(clientFd, $0.baseAddress, 4) }
    _ = response.withUnsafeBytes { write(clientFd, $0.baseAddress, response.count) }
  }
  close(clientFd)
  currentClientFd = nil
}

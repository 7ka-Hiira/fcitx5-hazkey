import Dispatch
import Foundation
import KanaKanjiConverterModule

// Parse command line arguments
let arguments = CommandLine.arguments
var replaceExisting = false

for arg in arguments {
  if arg == "-r" || arg == "--replace" {
    replaceExisting = true
    break
  }
}

let runtimeDir = ProcessInfo.processInfo.environment["XDG_RUNTIME_DIR"] ?? "/tmp"
let uid = getuid()
let socketPath = "\(runtimeDir)/hazkey_server.\(uid).sock"
let pidFilePath = "\(runtimeDir)/hazkey_server.\(uid).pid"

func removePidFile() {
  try? FileManager.default.removeItem(atPath: pidFilePath)
}

func terminateExistingServer(pid: pid_t) -> Bool {
  print("Terminating existing server with PID \(pid)...")

  // Send SIGTERM to gracefully terminate
  if kill(pid, SIGTERM) != 0 {
    print("Failed to send SIGTERM to existing server")
    return false
  }

  for attempt in 1...40 {  // 40 try * 0.1 sec
    usleep(100_000)  // 0.1 sec

    // Check if process is still running
    if kill(pid, 0) != 0 {
      print("Existing server terminated successfully")
      return true
    }

    if attempt == 20 {  // try SIGKILL
      print("Server didn't respond to SIGTERM, sending SIGKILL...")
      kill(pid, SIGKILL)
    }
  }

  // Final check
  if kill(pid, 0) == 0 {
    print("Failed to terminate existing server")
    return false
  }

  print("Existing server terminated")
  return true
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
      if replaceExisting {
        if !terminateExistingServer(pid: pid) {
          print("Failed to terminate existing server. Exiting.")
          exit(1)
        }
        try? FileManager.default.removeItem(atPath: pidFilePath)
      } else {
        print("Another hazkey_server is already running.")
        print("Use -r or --replace option to replace the existing server.")
        exit(0)
      }
    }
  }
  try? FileManager.default.removeItem(atPath: pidFilePath)
}

// create pid file
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
var config: KkcConfig
var composingText: ComposingTextBox = ComposingTextBox()
var currentCandidateList: [Candidate]? = nil

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

listenSocket()

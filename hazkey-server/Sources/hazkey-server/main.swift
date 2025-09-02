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
let socketPath = "\(runtimeDir)/hazkey-server.\(uid).sock"
let pidFilePath = "\(runtimeDir)/hazkey-server.\(uid).pid"

func removePidFile() {
    try? FileManager.default.removeItem(atPath: pidFilePath)
}

func terminateExistingServer(pid: pid_t) -> Bool {
    NSLog("Terminating existing server with PID \(pid)...")

    // Send SIGTERM to gracefully terminate
    if kill(pid, SIGTERM) != 0 {
        NSLog("Failed to send SIGTERM to existing server")
        return false
    }

    for attempt in 1...40 {  // 40 try * 0.1 sec
        usleep(100_000)  // 0.1 sec

        // Check if process is still running
        if kill(pid, 0) != 0 {
            NSLog("Existing server terminated successfully")
            return true
        }

        if attempt == 20 {  // try SIGKILL
            NSLog("Server didn't respond to SIGTERM, sending SIGKILL...")
            kill(pid, SIGKILL)
        }
    }

    // Final check
    if kill(pid, 0) == 0 {
        NSLog("Failed to terminate existing server")
        return false
    }

    NSLog("Existing server terminated")
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
    NSLog("SIGPIPE received - client disconnected")
}

if FileManager.default.fileExists(atPath: pidFilePath) {
    if let pidString = try? String(contentsOfFile: pidFilePath, encoding: .utf8),
        let pid = pid_t(pidString)
    {
        if kill(pid, 0) == 0 {
            if replaceExisting {
                if !terminateExistingServer(pid: pid) {
                    NSLog("Failed to terminate existing server. Exiting.")
                    exit(1)
                }
                try? FileManager.default.removeItem(atPath: pidFilePath)
            } else {
                NSLog("Another hazkey-server is already running.")
                NSLog("Use -r or --replace option to replace the existing server.")
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
let zenzaiAvailable = {
    guard let handle = dlopen(nil, RTLD_NOW) else {
        // cannot dlopen current process
        NSLog("Failed to dlopen current process")
        return false
    }
    defer { dlclose(handle) }
    // actual llama library shouldn't have this symbol
    return dlsym(handle, "llama_lib_is_stub") == nil
}()

let systemResourceDir = URL(fileURLWithPath: systemResourcePath, isDirectory: true)
var converter = KanaKanjiConverter.init(
    dictionaryURL: systemResourceDir.appendingPathComponent("Dictionary", isDirectory: true))
var profiles: [Hazkey_Config_Profile] = []
var composingText: ComposingTextBox = ComposingTextBox()
var currentCandidateList: [Candidate]?

do {
    profiles = try loadConfig()
} catch {
    NSLog("Failed to load config: \(error)")
    NSLog("Loading default config...")
    profiles = [genDefaultConfig()]
}

// TODO: add [0] out of range handling
var currentProfile = profiles[0]

var baseConvertRequestOptions = genBaseConvertRequestOptions()
// set non-blocking
var flags = fcntl(fd, F_GETFL, 0)
let fcntlRes = fcntl(fd, F_SETFL, flags | O_NONBLOCK)
if fcntlRes != 0 {
    NSLog("fcntl() failed")
}

NSLog("start listening...")

listenSocket()

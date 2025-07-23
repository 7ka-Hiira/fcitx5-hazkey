import Foundation
import KanaKanjiConverterModule

let runtimeDir = ProcessInfo.processInfo.environment["XDG_RUNTIME_DIR"] ?? "/tmp"
let uid = getuid()
let socketPath = "\(runtimeDir)/hazkey_server.\(uid).sock"

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
var composingText: ComposingText? = nil
var currentPreedit: String = ""

while true {
    var clientAddr = sockaddr()
    var clientLen: socklen_t = socklen_t(MemoryLayout<sockaddr>.size)
    let clientFd = accept(fd, &clientAddr, &clientLen)
    if clientFd == -1 {
        print("{\"status\": \"error\", \"message\": \"Accept failed\"}")
        continue
    }

    var buf = [UInt8](repeating: 0, count: 4096)
    let readBytes = read(clientFd, &buf, buf.count)
    if readBytes > 0 {
        let message = String(bytes: buf[0..<readBytes], encoding: .utf8) ?? ""
        print("Received: \(message)")

        let response = processJson(jsonString: message)

        _ = response.withCString { write(clientFd, $0, strlen($0)) }
    }
    close(clientFd)
}

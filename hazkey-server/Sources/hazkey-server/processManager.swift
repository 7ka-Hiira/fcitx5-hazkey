import Foundation

class ProcessManager {
    private let runtimeDir: String
    private let uid: uid_t
    private let pidFilePath: String
    private var replaceExisting: Bool = false

    init() {
        self.runtimeDir = ProcessInfo.processInfo.environment["XDG_RUNTIME_DIR"] ?? "/tmp"
        self.uid = getuid()
        self.pidFilePath = "\(runtimeDir)/hazkey-server.\(uid).pid"
    }

    func parseCommandLineArguments() {
        let arguments = CommandLine.arguments
        for arg in arguments {
            if arg == "-r" || arg == "--replace" {
                replaceExisting = true
                break
            }
        }
    }

    func setupSignalHandlers() {
        signal(SIGTERM) { _ in
            let runtimeDir = ProcessInfo.processInfo.environment["XDG_RUNTIME_DIR"] ?? "/tmp"
            let uid = getuid()
            let pidFilePath = "\(runtimeDir)/hazkey-server.\(uid).pid"
            try? FileManager.default.removeItem(atPath: pidFilePath)
            exit(0)
        }
        signal(SIGINT) { _ in
            let runtimeDir = ProcessInfo.processInfo.environment["XDG_RUNTIME_DIR"] ?? "/tmp"
            let uid = getuid()
            let pidFilePath = "\(runtimeDir)/hazkey-server.\(uid).pid"
            try? FileManager.default.removeItem(atPath: pidFilePath)
            exit(0)
        }
        signal(SIGPIPE) { _ in
            NSLog("SIGPIPE received - client disconnected")
        }
    }

    func checkExistingServer() throws {
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
    }

    func createPidFile() throws {
        try "\(getpid())".write(toFile: pidFilePath, atomically: true, encoding: .utf8)
    }

    func removePidFile() {
        try? FileManager.default.removeItem(atPath: pidFilePath)
    }

    private func terminateExistingServer(pid: pid_t) -> Bool {
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
}

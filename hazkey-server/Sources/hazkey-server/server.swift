import Dispatch
import Foundation
import KanaKanjiConverterModule

class HazkeyServer: SocketManagerDelegate {
    private let processManager: ProcessManager
    private let socketManager: SocketManager
    private let protocolHandler: ProtocolHandler
    private let zenzaiAvailable: Bool
    private let state: HazkeyServerState

    private let runtimeDir: String
    private let uid: uid_t
    private let socketPath: String

    init() {
        // Initialize runtime paths
        self.runtimeDir = ProcessInfo.processInfo.environment["XDG_RUNTIME_DIR"] ?? "/tmp"
        self.uid = getuid()
        self.socketPath = "\(runtimeDir)/hazkey-server.\(uid).sock"

        // Check if zenzai is available
        self.zenzaiAvailable = {
            guard let handle = dlopen(nil, RTLD_NOW) else {
                NSLog("Failed to dlopen current process")
                return false
            }
            defer { dlclose(handle) }
            return dlsym(handle, "llama_lib_is_stub") == nil
        }()

        // Initialize server state
        self.state = HazkeyServerState(zenzaiAvailable: zenzaiAvailable)

        // Initialize managers
        self.processManager = ProcessManager()
        self.socketManager = SocketManager(socketPath: socketPath)
        self.protocolHandler = ProtocolHandler(zenzaiAvailable: zenzaiAvailable, state: state)

        // Set delegate
        socketManager.delegate = self
    }

    func start() throws {
        processManager.parseCommandLineArguments()
        processManager.setupSignalHandlers()
        try processManager.checkExistingServer()
        try processManager.createPidFile()
        try socketManager.setupSocket()

        NSLog("start listening...")
        socketManager.startListening()
    }

    func socketManager(_ manager: SocketManager, didReceiveData data: Data, from clientFd: Int32)
        -> Data
    {
        return protocolHandler.processProto(data: data)
    }

    func socketManager(_ manager: SocketManager, clientDidConnect clientFd: Int32) {
        // Optional: Handle client connection events if needed
    }

    func socketManager(_ manager: SocketManager, clientDidDisconnect clientFd: Int32) {
        // Optional: Handle client disconnection events if needed
    }

    // Getter methods for backward compatibility (if needed elsewhere)
    var getZenzaiAvailable: Bool { zenzaiAvailable }
    var getState: HazkeyServerState { state }
}

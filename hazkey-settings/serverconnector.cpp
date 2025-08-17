#include "serverconnector.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <unistd.h>

#include <QMessageBox>
#include <mutex>
#include <thread>

static std::mutex transact_mutex;

ServerConnector::ServerConnector() {}

std::string ServerConnector::get_socket_path() {
    const char* xdg_runtime_dir = std::getenv("XDG_RUNTIME_DIR");
    uid_t uid = getuid();
    std::string sockname = "hazkey-server." + std::to_string(uid) + ".sock";
    if (xdg_runtime_dir && xdg_runtime_dir[0] != '\0') {
        return std::string(xdg_runtime_dir) + "/" + sockname;
    } else {
        return "/tmp/" + sockname;
    }
}

void ServerConnector::start_hazkey_server() {
    pid_t pid = fork();
    if (pid == 0) {
        // First child process
        pid_t second_pid = fork();

        if (second_pid == 0) {
            // TODO: get install dir
            execl(
                "/usr/lib"
                "/hazkey/hazkey-server",
                "hazkey-server", (char*)NULL);
            exit(1);
        } else if (second_pid < 0) {
            exit(1);
        } else {
            exit(0);
        }
    } else if (pid < 0) {
    } else {
        int status;
        waitpid(pid, &status, 0);
    }
}

bool writeAll(int fd, const void* data, size_t len) {
    size_t sent = 0;
    while (sent < len) {
        ssize_t n = write(fd, (const char*)data + sent, len - sent);
        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                fd_set wfds;
                FD_ZERO(&wfds);
                FD_SET(fd, &wfds);
                timeval tv = {2, 0};  // 2sec timeout
                int r = select(fd + 1, NULL, &wfds, NULL, &tv);
                if (r <= 0) {
                    return false;
                }
                continue;
            }
            return false;
        }
        sent += n;
    }
    return true;
}

bool readAll(int fd, void* data, size_t len) {
    size_t recved = 0;
    while (recved < len) {
        ssize_t n = read(fd, (char*)data + recved, len - recved);
        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                fd_set rfds;
                FD_ZERO(&rfds);
                FD_SET(fd, &rfds);
                timeval tv = {10, 0};  // 2sec timeout
                int r = select(fd + 1, &rfds, NULL, NULL, &tv);
                if (r <= 0) {
                    return false;
                }
                continue;
            }
            return false;
        }
        if (n == 0) return false;  // closed
        recved += n;
    }
    return true;
}

void ServerConnector::connect_server() {
    std::string socket_path = get_socket_path();

    constexpr int MAX_RETRIES = 3;
    constexpr int RETRY_INTERVAL_MS = 100;
    int attempt;
    for (attempt = 0; attempt < MAX_RETRIES; ++attempt) {
        sock_ = socket(AF_UNIX, SOCK_STREAM, 0);
        if (sock_ < 0) {
            std::this_thread::sleep_for(
                std::chrono::milliseconds(RETRY_INTERVAL_MS));
            continue;
        }
        int fcntlRes =
            fcntl(sock_, F_SETFL, fcntl(sock_, F_GETFL, 0) | O_NONBLOCK);
        if (fcntlRes != 0) {
            close(sock_);
            sock_ = -1;
            std::this_thread::sleep_for(
                std::chrono::milliseconds(RETRY_INTERVAL_MS));
            continue;
        }

        sockaddr_un addr{};
        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, socket_path.c_str(), sizeof(addr.sun_path) - 1);

        int ret = connect(sock_, (sockaddr*)&addr, sizeof(addr));
        if (ret == 0) {
            // Connected
            return;
        }
        if (errno == EINPROGRESS) {
            fd_set wfds;
            FD_ZERO(&wfds);
            FD_SET(sock_, &wfds);
            timeval tv = {2, 0};
            int sel = select(sock_ + 1, NULL, &wfds, NULL, &tv);
            if (sel > 0 && FD_ISSET(sock_, &wfds)) {
                int so_error = 0;
                socklen_t len = sizeof(so_error);
                getsockopt(sock_, SOL_SOCKET, SO_ERROR, &so_error, &len);
                if (so_error == 0) {
                    // Connected
                    return;
                }
            }
        }
        close(sock_);
        sock_ = -1;
        start_hazkey_server();
        std::this_thread::sleep_for(
            std::chrono::milliseconds(RETRY_INTERVAL_MS));
    }
}

std::optional<hazkey::ResponseEnvelope> ServerConnector::transact(
    const hazkey::RequestEnvelope& send_data) {
    std::lock_guard<std::mutex> lock(transact_mutex);

    if (sock_ == -1) {
        connect_server();
        if (sock_ == -1) {
            return std::nullopt;
        }
    }

    std::string msg;
    if (!send_data.SerializeToString(&msg)) {
        return std::nullopt;
    }
    // write length
    uint32_t writeLen = htonl(msg.size());
    if (!writeAll(sock_, &writeLen, 4)) {
        close(sock_);
        sock_ = -1;
        connect_server();
        return std::nullopt;
    }

    // write data
    if (!writeAll(sock_, msg.c_str(), msg.size())) {
        close(sock_);
        sock_ = -1;
        connect_server();
        return std::nullopt;
    }

    // read response length
    uint32_t readLenBuf;
    if (!readAll(sock_, &readLenBuf, 4)) {
        close(sock_);
        sock_ = -1;
        return std::nullopt;
    }

    uint32_t readLen = ntohl(readLenBuf);

    if (readLen > 2 * 1024 * 1024) {  // 2MB limit
        close(sock_);
        sock_ = -1;
        return std::nullopt;
    }

    // read response
    std::vector<char> buf(readLen);
    if (!readAll(sock_, buf.data(), readLen)) {
        close(sock_);
        sock_ = -1;
        return std::nullopt;
    }

    hazkey::ResponseEnvelope resp;
    if (!resp.ParseFromArray(buf.data(), readLen)) {
        return std::nullopt;
    }

    return resp;
}

std::optional<hazkey::config::CurrentConfig>
ServerConnector::getCurrentConfig() {
    hazkey::RequestEnvelope request;
    auto props = request.mutable_get_current_config();
    auto response = transact(request);
    if (response == std::nullopt) {
        return std::nullopt;
    }
    auto responseVal = response.value();
    if (responseVal.status() != hazkey::SUCCESS) {
        return std::nullopt;
    }
    if (!responseVal.has_current_config()) {
        return std::nullopt;
    }
    return responseVal.current_config();
}

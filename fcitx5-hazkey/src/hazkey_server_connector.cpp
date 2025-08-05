#include "hazkey_server_connector.h"

#include <arpa/inet.h>
#include <fcitx-utils/log.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <unistd.h>

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <mutex>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <thread>

#include "env_config.h"
#include "protocol/hazkey_server.pb.h"

static std::mutex transact_mutex;

std::string HazkeyServerConnector::get_socket_path() {
    const char* xdg_runtime_dir = std::getenv("XDG_RUNTIME_DIR");
    uid_t uid = getuid();
    std::string sockname = "hazkey_server." + std::to_string(uid) + ".sock";
    if (xdg_runtime_dir && xdg_runtime_dir[0] != '\0') {
        return std::string(xdg_runtime_dir) + "/" + sockname;
    } else {
        return "/tmp/" + sockname;
    }
}

void HazkeyServerConnector::start_hazkey_server() {
    pid_t pid = fork();
    FCITX_DEBUG() << "First fork PID: " << pid;
    if (pid == 0) {
        // First child process
        pid_t second_pid = fork();
        FCITX_DEBUG() << "Second fork PID: " << second_pid;

        if (second_pid == 0) {
            // Grandchild process - this will run the actual server
            execl(INSTALL_LIBDIR "/hazkey/hazkey_server", "hazkey_server",
                  (char*)NULL);
            FCITX_ERROR() << "Failed to start hazkey_server\n";
            exit(1);
        } else if (second_pid < 0) {
            FCITX_ERROR() << "Failed to create second fork\n";
            exit(1);
        } else {
            exit(0);
        }
    } else if (pid < 0) {
        FCITX_ERROR() << "Failed to start hazkey_server (first fork failed)\n";
    } else {
        int status;
        waitpid(pid, &status, 0);
        FCITX_DEBUG() << "First child exited with status: " << status;
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
                    FCITX_ERROR() << "write timeout";
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
                    FCITX_ERROR() << "read timeout";
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

void HazkeyServerConnector::connect_server() {
    std::string socket_path = get_socket_path();

    constexpr int MAX_RETRIES = 3;
    constexpr int RETRY_INTERVAL_MS = 100;
    int attempt;
    for (attempt = 0; attempt < MAX_RETRIES; ++attempt) {
        sock_ = socket(AF_UNIX, SOCK_STREAM, 0);
        if (sock_ < 0) {
            FCITX_ERROR() << "Failed to create socket";
            std::this_thread::sleep_for(
                std::chrono::milliseconds(RETRY_INTERVAL_MS));
            continue;
        }
        int fcntlRes =
            fcntl(sock_, F_SETFL, fcntl(sock_, F_GETFL, 0) | O_NONBLOCK);
        if (fcntlRes != 0) {
            FCITX_ERROR() << "fcntl() failed";
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
        FCITX_INFO() << "Failed to connect hazkey_server, retry "
                     << (attempt + 1);
        close(sock_);
        sock_ = -1;
        start_hazkey_server();
        std::this_thread::sleep_for(
            std::chrono::milliseconds(RETRY_INTERVAL_MS));
    }
    FCITX_INFO() << "Failed to connect hazkey_server after " << MAX_RETRIES
                 << " attempts";
}

std::optional<hazkey::commands::ResultData> HazkeyServerConnector::transact(
    const hazkey::commands::QueryData& send_data) {
    std::lock_guard<std::mutex> lock(transact_mutex);

    if (sock_ == -1) {
        FCITX_INFO() << "Socket not connected, attempting to connect...";
        connect_server();
        if (sock_ == -1) {
            FCITX_ERROR() << "Failed to establish connection to hazkey_server";
            return std::nullopt;
        }
    }

    std::string msg;
    if (!send_data.SerializeToString(&msg)) {
        FCITX_ERROR() << "Failed to serialize protobuf message.";
        return std::nullopt;
    }

    FCITX_DEBUG() << "Sending message of size: " << msg.size();

    // write length
    uint32_t writeLen = htonl(msg.size());
    if (!writeAll(sock_, &writeLen, 4)) {
        FCITX_INFO()
            << "Failed to communicate with server while writing data length. "
               "restarting hazkey_server...";
        close(sock_);
        sock_ = -1;
        connect_server();
        return std::nullopt;
    }

    // write data
    if (!writeAll(sock_, msg.c_str(), msg.size())) {
        FCITX_INFO() << "Failed to communicate with server while writing data. "
                        "restarting hazkey_server...";
        close(sock_);
        sock_ = -1;
        connect_server();
        return std::nullopt;
    }

    FCITX_DEBUG() << "Successfully wrote data to server";

    // read response length
    uint32_t readLenBuf;
    if (!readAll(sock_, &readLenBuf, 4)) {
        FCITX_ERROR() << "Failed to read buffer length.";
        close(sock_);
        sock_ = -1;
        return std::nullopt;
    }

    uint32_t readLen = ntohl(readLenBuf);
    FCITX_DEBUG() << "Server response size: " << readLen;

    if (readLen > 2 * 1024 * 1024) {  // 2MB limit
        FCITX_ERROR() << "Response size too large: " << readLen;
        close(sock_);
        sock_ = -1;
        return std::nullopt;
    }

    std::vector<char> buf(readLen);
    if (!readAll(sock_, buf.data(), readLen)) {
        FCITX_ERROR() << "Failed to read response body.";
        close(sock_);
        sock_ = -1;
        return std::nullopt;
    }

    hazkey::commands::ResultData resp;
    if (!resp.ParseFromArray(buf.data(), readLen)) {
        FCITX_ERROR() << "Failed to parse received data\n";
        return std::nullopt;
    }

    FCITX_DEBUG() << "Successfully received and parsed response";
    return resp;
}

std::string HazkeyServerConnector::getComposingText(
    hazkey::commands::QueryData::GetComposingStringProps::CharType type) {
    hazkey::commands::QueryData query;
    query.set_function(hazkey::commands::QueryData_KkcApi::
                           QueryData_KkcApi_GET_COMPOSING_STRING);
    auto props = query.mutable_get_composing_string();
    props->set_char_type(type);
    auto response = transact(query);
    if (response == std::nullopt) {
        FCITX_ERROR() << "Error while transacting getComposingText().";
        return "";
    }
    auto responseVal = response.value();
    if (responseVal.status() != hazkey::commands::ResultData::SUCCESS) {
        FCITX_ERROR() << "Server returned error: "
                      << responseVal.errormessage();
        return "";
    }
    return responseVal.result();
}

std::string HazkeyServerConnector::getComposingHiraganaWithCursor() {
    hazkey::commands::QueryData query;
    query.set_function(hazkey::commands::QueryData_KkcApi::
                           QueryData_KkcApi_GET_HIRAGANA_WITH_CURSOR);
    auto response = transact(query);
    if (response == std::nullopt) {
        FCITX_ERROR()
            << "Error while transacting getComposingHiraganaWithCursor().";
        return "";
    }
    auto responseVal = response.value();
    if (responseVal.status() != hazkey::commands::ResultData::SUCCESS) {
        FCITX_ERROR() << "Server returned error: "
                      << responseVal.errormessage();
        return "";
    }
    return responseVal.result();
}

void HazkeyServerConnector::addToComposingText(std::string text,
                                               bool isDirect) {
    hazkey::commands::QueryData query;
    query.set_function(
        hazkey::commands::QueryData_KkcApi::QueryData_KkcApi_INPUT_TEXT);
    auto props = query.mutable_input_text();
    props->set_text(text);
    props->set_is_direct(isDirect);
    auto response = transact(query);
    if (response == std::nullopt) {
        FCITX_ERROR() << "Error while transacting addToComposingText().";
        return;
    }
    auto responseVal = response.value();
    if (responseVal.status() != hazkey::commands::ResultData::SUCCESS) {
        FCITX_ERROR() << "Server returned error: "
                      << responseVal.errormessage();
        return;
    }
    return;
}

void HazkeyServerConnector::deleteLeft() {
    hazkey::commands::QueryData query;
    query.set_function(
        hazkey::commands::QueryData_KkcApi::QueryData_KkcApi_DELETE_LEFT);
    auto response = transact(query);
    if (response == std::nullopt) {
        FCITX_ERROR() << "Error while transacting deleteLeft().";
        return;
    }
    auto responseVal = response.value();
    if (responseVal.status() != hazkey::commands::ResultData::SUCCESS) {
        FCITX_ERROR() << "Server returned error: "
                      << responseVal.errormessage();
        return;
    }
    return;
}

void HazkeyServerConnector::deleteRight() {
    hazkey::commands::QueryData query;
    query.set_function(
        hazkey::commands::QueryData_KkcApi::QueryData_KkcApi_DELETE_RIGHT);
    auto response = transact(query);
    if (response == std::nullopt) {
        FCITX_ERROR() << "Error while transacting deleteRight().";
        return;
    }
    auto responseVal = response.value();
    if (responseVal.status() != hazkey::commands::ResultData::SUCCESS) {
        FCITX_ERROR() << "Server returned error: "
                      << responseVal.errormessage();
        return;
    }
    return;
}

void HazkeyServerConnector::moveCursor(int offset) {
    hazkey::commands::QueryData query;
    query.set_function(
        hazkey::commands::QueryData_KkcApi::QueryData_KkcApi_MOVE_CURSOR);
    auto props = query.mutable_move_cursor();
    props->set_offset(offset);
    auto response = transact(query);
    if (response == std::nullopt) {
        FCITX_ERROR() << "Error while transacting moveCursor().";
        return;
    }
    auto responseVal = response.value();
    if (responseVal.status() != hazkey::commands::ResultData::SUCCESS) {
        FCITX_ERROR() << "Server returned error: "
                      << responseVal.errormessage();
        return;
    }
    return;
}

void HazkeyServerConnector::setLeftContext(std::string context, int anchor) {
    hazkey::commands::QueryData query;
    query.set_function(
        hazkey::commands::QueryData_KkcApi::QueryData_KkcApi_SET_LEFT_CONTEXT);
    auto props = query.mutable_set_left_context();
    props->set_context(context);
    props->set_anchor(anchor);
    auto response = transact(query);
    if (response == std::nullopt) {
        FCITX_ERROR() << "Error while transacting setLeftContext().";
        return;
    }
    auto responseVal = response.value();
    if (responseVal.status() != hazkey::commands::ResultData::SUCCESS) {
        FCITX_ERROR() << "Server returned error: "
                      << responseVal.errormessage();
        return;
    }
    return;
}

void HazkeyServerConnector::setServerConfig(
    int zenzaiEnabled, int zenzaiInferLimit, int numberFullwidth,
    int symbolFullwidth, int periodStyleIndex, int commaStyleIndex,
    int spaceFullwidth, int tenCombining, std::string profileText) {
    hazkey::commands::QueryData query;
    query.set_function(
        hazkey::commands::QueryData_KkcApi::QueryData_KkcApi_SET_CONFIG);
    auto props = query.mutable_set_config();
    props->set_zenzai_enabled(zenzaiEnabled);
    props->set_zenzai_infer_limit(zenzaiInferLimit);
    props->set_number_fullwidth(numberFullwidth);
    props->set_symbol_fullwidth(symbolFullwidth);
    props->set_period_style(periodStyleIndex);
    props->set_comma_style(commaStyleIndex);
    props->set_space_fullwidth(spaceFullwidth);
    props->set_ten_combining(tenCombining);
    props->set_profile_text(profileText);
    auto response = transact(query);
    if (response == std::nullopt) {
        FCITX_ERROR() << "Error while transacting setServerConfig().";
        return;
    }
    auto responseVal = response.value();
    if (responseVal.status() != hazkey::commands::ResultData::SUCCESS) {
        FCITX_ERROR() << "Server returned error: "
                      << responseVal.errormessage();
        return;
    }
    return;
}

void HazkeyServerConnector::createComposingTextInstance() {
    hazkey::commands::QueryData query;
    query.set_function(hazkey::commands::QueryData_KkcApi::
                           QueryData_KkcApi_CREATE_COMPOSING_TEXT_INSTANCE);
    auto response = transact(query);
    if (response == std::nullopt) {
        FCITX_ERROR()
            << "Error while transacting createComposingTextInstance().";
        return;
    }
    auto responseVal = response.value();
    if (responseVal.status() != hazkey::commands::ResultData::SUCCESS) {
        FCITX_ERROR() << "Server returned error: "
                      << responseVal.errormessage();
        return;
    }
    return;
}

void HazkeyServerConnector::completePrefix(int index) {
    hazkey::commands::QueryData query;
    query.set_function(
        hazkey::commands::QueryData_KkcApi::QueryData_KkcApi_COMPLETE_PREFIX);
    auto props = query.mutable_complete_prefix();
    props->set_index(index);
    auto response = transact(query);
    if (response == std::nullopt) {
        FCITX_ERROR() << "Error while transacting completePrefix().";
        return;
    }
    auto responseVal = response.value();
    if (responseVal.status() != hazkey::commands::ResultData::SUCCESS) {
        FCITX_ERROR() << "Server returned error: "
                      << responseVal.errormessage();
        return;
    }
    return;
}

std::vector<HazkeyServerConnector::CandidateData>
HazkeyServerConnector::getCandidates(bool isPredictMode, int n_best) {
    hazkey::commands::QueryData query;
    query.set_function(
        hazkey::commands::QueryData_KkcApi::QueryData_KkcApi_GET_CANDIDATES);
    auto props = query.mutable_get_candidates();
    props->set_is_predict_mode(isPredictMode);
    props->set_n_best(n_best);
    auto response = transact(query);
    if (response == std::nullopt) {
        FCITX_ERROR() << "Error while transacting setServerConfig().";
        std::vector<CandidateData> empty_vec;
        return empty_vec;
    }
    auto responseVal = response.value();
    if (responseVal.status() != hazkey::commands::ResultData::SUCCESS) {
        FCITX_ERROR() << "Server returned error: "
                      << responseVal.errormessage();
        std::vector<CandidateData> empty_vec;
        return empty_vec;
    }
    std::vector<CandidateData> candidates;
    for (const auto& item : responseVal.candidates().candidates()) {
        CandidateData candidate =
            CandidateData(item.text(), item.sub_hiragana());
        candidates.push_back(candidate);
    }
    return candidates;
}

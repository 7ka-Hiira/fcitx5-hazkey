#include "hazkey_server_connector.h"

#include <fcitx-utils/log.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <mutex>
#include <nlohmann/json.hpp>
#include <string>
#include <thread>

#include "env_config.h"
#include "hazkey_engine.h"

static std::mutex transact_mutex;

using json = nlohmann::json;

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
    FCITX_DEBUG() << "PID: " << pid;
    if (pid == 0) {
        // TODO: use build time var & environment var
        execl(INSTALL_LIBDIR "/hazkey/hazkey_server", "hazkey_server",
              (char*)NULL);
        FCITX_ERROR() << "Failed to start hazkey_server\n";
        exit(1);
    } else if (pid < 0) {
        std::cerr << "Failed to start hazkey_server\n";
    }
}

void HazkeyServerConnector::connect_server() {
    std::string socket_path = get_socket_path();

    start_hazkey_server();

    sock_ = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock_ < 0) {
        FCITX_DEBUG() << "Failed to connect hazkey_server";
        return;
    }

    sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path.c_str(), sizeof(addr.sun_path) - 1);

    constexpr int MAX_RETRIES = 10;
    constexpr int RETRY_INTERVAL_MS = 100;
    int attempt;
    for (attempt = 0; attempt < MAX_RETRIES; ++attempt) {
        std::this_thread::sleep_for(
            std::chrono::milliseconds(RETRY_INTERVAL_MS));
        if (connect(sock_, (sockaddr*)&addr, sizeof(addr)) == 0) {
            return;
        }
        FCITX_INFO() << "Failed to connect hazkey_server, retry "
                     << (attempt + 1);
    }
    close(sock_);
    FCITX_INFO() << "Failed to connect hazkey_server after " << MAX_RETRIES
                 << " attempts";
    return;
}

json HazkeyServerConnector::transact(const json& send_data) {
    std::lock_guard<std::mutex> lock(transact_mutex);
    std::string msg = send_data.dump();

    if (write(sock_, msg.c_str(), msg.size()) < 0) {
        FCITX_INFO() << "Failed to communicate with server while writing data. "
                        "restarting hazkey_server...";
        connect_server();
        return nullptr;
    }

    // TODO: Iterating when data is longer than buffer
    char buf[65536];
    int len = read(sock_, buf, sizeof(buf));
    if (len < 0) {
        FCITX_INFO() << "Failed to communicate with server while reading data";
        return nullptr;
    }
    std::string received(buf, len);

    json resp = json::parse(received, nullptr, false);
    if (resp.is_discarded()) {
        std::cerr << "Failed to parse received JSON\n";
        return nullptr;
    }
    return resp;
}

std::string HazkeyServerConnector::getComposingText(std::string type) {
    nlohmann::json req = {{"function", "get_composing_string"},
                          {"props", {{"char_type", type}}}};
    nlohmann::json resp = transact(req);
    if (!resp.is_object() || !resp.contains("result")) {
        return "";
    }
    return resp["result"].get<std::string>();
}

std::string HazkeyServerConnector::getComposingHiraganaWithCursor() {
    nlohmann::json req = {{"function", "get_hiragana_with_cursor"}};
    nlohmann::json resp = transact(req);
    if (!resp.is_object() || !resp.contains("result")) {
        return "";
    }
    return resp["result"].get<std::string>();
}

void HazkeyServerConnector::addToComposingText(std::string text,
                                               bool isDirect) {
    nlohmann::json req = {{"function", "input_text"},
                          {"props", {{"text", text}, {"is_direct", isDirect}}}};
    transact(req);
}

void HazkeyServerConnector::deleteLeft() {
    nlohmann::json req = {{"function", "delete_left"}};
    transact(req);
}

void HazkeyServerConnector::deleteRight() {
    nlohmann::json req = {{"function", "delete_right"}};
    transact(req);
}

void HazkeyServerConnector::moveCursor(int offset) {
    nlohmann::json req = {{"function", "move_cursor"},
                          {"props", {{"offset", offset}}}};
    transact(req);
}

void HazkeyServerConnector::setLeftContext(std::string context, int anchor) {
    nlohmann::json req = {
        {"function", "input_text"},
        {"props", {{"context", context}, {"anchor", anchor}}}};
    transact(req);
}

void HazkeyServerConnector::setServerConfig(
    int zenzaiEnabled, int zenzaiInferLimit, int numberFullwidth,
    int symbolFullwidth, int periodStyleIndex, int commaStyleIndex,
    int spaceFullwidth, int tenCombining, std::string profileText) {
    nlohmann::json req = {{"function", "set_config"},
                          {"props",
                           {{"zenzai_enabled", zenzaiEnabled != 0},
                            {"zenzai_infer_limit", zenzaiInferLimit},
                            {"number_fullwidth", numberFullwidth},
                            {"symbol_fullwidth", symbolFullwidth},
                            {"period_style", periodStyleIndex},
                            {"comma_style", commaStyleIndex},
                            {"space_fullwidth", spaceFullwidth},
                            {"ten_combining", tenCombining},
                            {"profile_text", profileText}}}};
    transact(req);
}

void HazkeyServerConnector::createComposingTextInstance() {
    nlohmann::json req = {{"function", "create_composing_text_instance"}};
    transact(req);
}

void HazkeyServerConnector::completePrefix() {
    nlohmann::json req = {{"function", "complete_prefix"}};
    transact(req);
}

std::vector<HazkeyServerConnector::CandidateData>
HazkeyServerConnector::getCandidates(bool isPredictMode, int n_best) {
    nlohmann::json req = {
        {"function", "get_candidates"},
        {"props", {{"is_predict_mode", isPredictMode}, {"n_best", n_best}}}};
    nlohmann::json res = transact(req);

    std::vector<CandidateData> candidates;
    for (const auto& item : res) {
        if (item.contains("t") && item["t"].is_string() && item.contains("h") &&
            item["h"].is_string()) {
            CandidateData candidate = CandidateData(
                item["t"].get<std::string>(), item["h"].get<std::string>());
            candidates.push_back(candidate);
        }
    }
    return candidates;
}

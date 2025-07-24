#include <fcitx-utils/log.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>
#include <thread>

#include "env_config.h"
#include "hazkey_engine.h"

using json = nlohmann::json;

std::string get_socket_path() {
    const char* xdg_runtime_dir = std::getenv("XDG_RUNTIME_DIR");
    uid_t uid = getuid();
    std::string sockname = "hazkey_server." + std::to_string(uid) + ".sock";
    if (xdg_runtime_dir && xdg_runtime_dir[0] != '\0') {
        return std::string(xdg_runtime_dir) + "/" + sockname;
    } else {
        return "/tmp/" + sockname;
    }
}

void start_hazkey_server() {
    pid_t pid = fork();
    FCITX_DEBUG() << "PID: " << pid;
    if (pid == 0) {
        // TODO: use build time var & environment var
        execl("/usr/lib/hazkey/hazkey_server", "hazkey_server", (char*)NULL);
        FCITX_ERROR() << "Failed to start hazkey_server\n";
        exit(1);
    } else if (pid < 0) {
        std::cerr << "Failed tto start hazkey_server\n";
    }
}

int connect_server() {
    std::string socket_path = get_socket_path();

    start_hazkey_server();

    int sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock < 0) {
        FCITX_DEBUG() << "Failed to connect hazkey_server";
        return -1;
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
        if (connect(sock, (sockaddr*)&addr, sizeof(addr)) == 0) {
            return sock;
        }
        FCITX_INFO() << "Failed to connect hazkey_server, retry "
                     << (attempt + 1);
    }
    close(sock);
    FCITX_INFO() << "Failed to connect hazkey_server after " << MAX_RETRIES
                 << " attempts";
    return -1;
}

json transact(int sock, const json& send_data) {
    std::string msg = send_data.dump();

    if (write(sock, msg.c_str(), msg.size()) < 0) {
        FCITX_INFO() << "Failed to communicate with server while writing data";
        return nullptr;
    }

    char buf[4096];
    int len = read(sock, buf, sizeof(buf));
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

std::string getComposingText(int sock, std::string type) {
    nlohmann::json req = {{"function", "get_composing_string"},
                          {"props", {{"char_type", type}}}};
    nlohmann::json resp = transact(sock, req);
    if (!resp.is_object() || !resp.contains("result")) {
        return "";
    }
    return resp["result"].get<std::string>();
}

std::string getComposingHiraganaWithCursor(int sock) {
    nlohmann::json req = {{"function", "get_hiragana_with_cursor"}};
    nlohmann::json resp = transact(sock, req);
    if (!resp.is_object() || !resp.contains("result")) {
        return "";
    }
    return resp["result"].get<std::string>();
}

void addToComposingText(int sock, std::string text, bool isDirect) {
    nlohmann::json req = {{"function", "input_text"},
                          {"props", {{"text", text}, {"is_direct", isDirect}}}};
    transact(sock, req);
}

void deleteLeft(int sock) {
    nlohmann::json req = {{"function", "delete_left"}};
    transact(sock, req);
}

void deleteRight(int sock) {
    nlohmann::json req = {{"function", "delete_right"}};
    transact(sock, req);
}

void moveCursor(int sock, int offset) {
    nlohmann::json req = {{"function", "move_cursor"},
                          {"props", {{"offset", offset}}}};
    transact(sock, req);
}

void setLeftContext(int sock, std::string context, int anchor) {
    nlohmann::json req = {
        {"function", "input_text"},
        {"props", {{"context", context}, {"anchor", anchor}}}};
    transact(sock, req);
}

void setServerConfig(int sock, int zenzaiEnabled, int zenzaiInferLimit,
                     int numberFullwidth, int symbolFullwidth,
                     int periodStyleIndex, int commaStyleIndex,
                     int spaceFullwidth, int tenCombining,
                     std::string profileText) {
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
    transact(sock, req);
}

void createComposingTextInstance(int sock) {
    nlohmann::json req = {{"function", "create_composing_text_instance"}};
    transact(sock, req);
}

void completePrefix(int sock) {
    nlohmann::json req = {{"function", "complete_prefix"}};
    transact(sock, req);
}

std::vector<std::string> getServerCandidates(int sock, bool isPredictMode,
                                             int n_best) {
    nlohmann::json req = {
        {"function", "get_candidates"},
        {"props", {{"is_predict_mode", isPredictMode}, {"n_best", n_best}}}};
    nlohmann::json res = transact(sock, req);

    std::vector<std::string> candidates;
    for (const auto& item : res) {
        if (item.contains("candidateText") &&
            item["candidateText"].is_string()) {
            candidates.push_back(item["candidateText"].get<std::string>());
        }
    }
    return candidates;
}

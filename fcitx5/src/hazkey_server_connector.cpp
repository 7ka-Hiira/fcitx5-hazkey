#include <fcitx-utils/log.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <cstdlib>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>

#include "env_config.h"

using json = nlohmann::json;

std::string get_socket_path() {
    const char* xdg_runtime_dir = std::getenv("XDG_RUNTIME_DIR");
    uid_t uid = getuid();
    std::string sockname = "sample_server." + std::to_string(uid) + ".sock";
    if (xdg_runtime_dir && xdg_runtime_dir[0] != '\0') {
        return std::string(xdg_runtime_dir) + "/" + sockname;
    } else {
        return "/tmp/" + sockname;
    }
}

int connect_server() {
    std::string socket_path = get_socket_path();

    int sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock < 0) {
        FCITX_DEBUG() << "Failed to connect hazkey_server";
        return -1;
    }

    sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path.c_str(), sizeof(addr.sun_path) - 1);

    if (connect(sock, (sockaddr*)&addr, sizeof(addr)) < 0) {
        if (connect(sock, (sockaddr*)&addr, sizeof(addr)) < 0) {
            FCITX_INFO() << "Failed to connect hazkey_server";
            close(sock);
            return -1;
        }

        return sock;
    }
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

std::string getComposingText(int sock) {
    std::string rawJson = R"({"function": "get_composing_string"})";

    nlohmann::json req = nlohmann::json::parse(rawJson, nullptr, false);
    nlohmann::json resp = transact(sock, req);
    if (!resp.is_object() || !resp.contains("composing_string")) {
        return "";
    }
    return resp["composing_string"].get<std::string>();
}

void addToComposingText(int sock, std::string text, bool isDirect) {
  std::string rawFuncProps = "{\"text\": \"" + text + "\",\"isDirect\": \"" + std::string(isDirect ? "true" : "false") + "\"}";
  std::string rawJson = "{\"function\": \"input_text\",\"props\": \""+ rawFuncProps +"\"}";
  nlohmann::json req = nlohmann::json::parse(rawJson, nullptr, false);
  nlohmann::json resp = transact(sock, req);
  return;
}

void deleteLeft(int sock) {
  std::string rawJson = "{\"function\": \"delete_left\"}";
  nlohmann::json req = nlohmann::json::parse(rawJson, nullptr, false);
  nlohmann::json resp = transact(sock, req);
  return;
}

void deleteRight(int sock) {
  std::string rawJson = "{\"function\": \"delete_right\"}";
  nlohmann::json req = nlohmann::json::parse(rawJson, nullptr, false);
  nlohmann::json resp = transact(sock, req);
  return;
}

void moveCursor(int sock, int offset) {
  std::string rawFuncProps = "{\"offset\": \"" + to_string(offset) + "\"}";
  std::string rawJson = "{\"function\": \"move_cursor\",\"props\": \""+ rawFuncProps +"\"}";
  nlohmann::json req = nlohmann::json::parse(rawJson, nullptr, false);
  nlohmann::json resp = transact(sock, req);
  return;
}

void setLeftContext(int sock, std::string context, int anchor) {
  std::string rawFuncProps = "{\"context\": \"" + context + ",\"anchor\":\"" + to_string(anchor) + "\"}";
  std::string rawJson = "{\"function\": \"input_text\",\"props\": \""+ rawFuncProps +"\"}";
  nlohmann::json req = nlohmann::json::parse(rawJson, nullptr, false);
  nlohmann::json resp = transact(sock, req);
  return;
}

#include <fcitx-utils/log.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <cstdlib>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>

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

int connect_server() {
    FCITX_DEBUG() << "Hello1";
    std::string socket_path = get_socket_path();

    FCITX_DEBUG() << "Hello2";

    int sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock < 0) {
        FCITX_DEBUG() << "Failed to connect hazkey_server";
        return -1;
    }
    FCITX_DEBUG() << "Hello3";

    sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path.c_str(), sizeof(addr.sun_path) - 1);
    FCITX_DEBUG() << "Hello4";
    if (connect(sock, (sockaddr*)&addr, sizeof(addr)) < 0) {
        FCITX_INFO() << "Failed to connect hazkey_server";
        close(sock);
        return -1;
    }

    return sock;
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
    if (!resp.is_object() || !resp.contains("composing_string")) {
        return "";
    }
    return resp["composing_string"].get<std::string>();
}

std::string getComposingHiraganaWithCursor(int sock) {
    nlohmann::json req = {{"function", "get_hiragana_with_cursor"}};
    nlohmann::json resp = transact(sock, req);
    if (!resp.is_object() || !resp.contains("composing_string")) {
        return "";
    }
    return resp["composing_string"].get<std::string>();
}

void addToComposingText(int sock, std::string text, bool isDirect) {
    nlohmann::json req = {{"function", "input_text"},
                          {"props", {{"text", text}, {"isDirect", isDirect}}}};
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

void setConfig(int sock) {
    nlohmann::json req = {
        {"function", "set_config"},
    };
    transact(sock, req);
}

void createComposingTextInstance(int sock) {
    nlohmann::json req = {
        {"function", "create_composing_text_instance"},
    };
    transact(sock, req);
}

void completePrefix(int sock) {
    nlohmann::json req = {
        {"function", "complete_prefix"},
    };
    transact(sock, req);
}

json getServerCandidates(int sock) {
    nlohmann::json req = {
        {"function", "get_candidates"},
    };
    return transact(sock, req);
}

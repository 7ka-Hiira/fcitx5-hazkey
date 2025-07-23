#include "hazkey_server_connector.h"

#include <fcitx-utils/log.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <chrono>
#include <cstdlib>
#include <iostream>
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

std::optional<hazkey::commands::ResultData> HazkeyServerConnector::transact(
    const hazkey::commands::QueryData& send_data) {
    std::lock_guard<std::mutex> lock(transact_mutex);

    std::string msg;
    if (!send_data.SerializeToString(&msg)) {
        FCITX_ERROR() << "Failed to serialize protobuf message.";
        return std::nullopt;
    }

    if (write(sock_, msg.c_str(), msg.size()) < 0) {
        FCITX_INFO() << "Failed to communicate with server while writing data. "
                        "restarting hazkey_server...";
        connect_server();
        return std::nullopt;
    }

    // TODO: Iterating when data is longer than buffer
    char buf[65536];
    int len = read(sock_, buf, sizeof(buf));
    if (len < 0) {
        FCITX_INFO() << "Failed to communicate with server while reading data";
        return std::nullopt;
    }

    hazkey::commands::ResultData resp;
    if (!resp.ParseFromArray(buf, len)) {
        FCITX_ERROR() << "Failed to parse received JSON\n";
        return std::nullopt;
    }
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

void HazkeyServerConnector::completePrefix() {
    hazkey::commands::QueryData query;
    query.set_function(
        hazkey::commands::QueryData_KkcApi::QueryData_KkcApi_COMPLETE_PREFIX);
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

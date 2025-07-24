#ifndef HAZKEY_SERVER_CONNECTOR_H
#define HAZKEY_SERVER_CONNECTOR_H

#include <fcitx-utils/log.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#include "env_config.h"

using json = nlohmann::json;

class HazkeyServerConnector {
   public:
    // HazkeyServerConnector();
    // ~HazkeyServerConnector();

    HazkeyServerConnector() { connect_server(); }

    std::string get_socket_path();

    void connect_server();

    json transact(const json& send_data);

    std::string getComposingText(std::string type);

    std::string getComposingHiraganaWithCursor();

    void addToComposingText(std::string text, bool isDirect);

    void deleteLeft();

    void deleteRight();

    void moveCursor(int offset);

    void setLeftContext(std::string context, int anchor);

    void setServerConfig(int zenzaiEnabled, int zenzaiInferLimit,
                         int numberFullwidth, int symbolFullwidth,
                         int periodStyleIndex, int commaStyleIndex,
                         int spaceFullwidth, int tenCombining,
                         std::string profileText);

    void createComposingTextInstance();

    void completePrefix();

    struct CandidateData {
        std::string candidateText;
        std::string subHiragana;
    };

    std::vector<CandidateData> getCandidates(bool isPredictMode, int n_best);

   private:
    void start_hazkey_server();
    int sock_;
    std::string socket_path_;
};

#endif  // HAZKEY_SERVER_CONNECTOR_H

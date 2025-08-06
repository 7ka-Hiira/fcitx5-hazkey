#ifndef HAZKEY_SERVER_CONNECTOR_H
#define HAZKEY_SERVER_CONNECTOR_H

#include <fcitx-utils/log.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <string>
#include <vector>

#include "env_config.h"
#include "protocol/base.pb.h"

class HazkeyServerConnector {
   public:
    // HazkeyServerConnector();
    // ~HazkeyServerConnector();

    HazkeyServerConnector() {
        // start_hazkey_server();
        connect_server();
        FCITX_DEBUG() << "Connector initialized";
    };

    std::string get_socket_path();

    void connect_server();

    std::optional<hazkey::ResponseEnvelope> transact(
        const hazkey::RequestEnvelope& send_data);

    std::string getComposingText(
        hazkey::commands::GetComposingString::CharType type,
        std::string currentPreedit);

    std::string getComposingHiraganaWithCursor();

    void inputChar(std::string text, bool isDirect);

    void deleteLeft();

    void deleteRight();

    void moveCursor(int offset);

    void setContext(std::string context, int anchor);

    void setServerConfig(int zenzaiEnabled, int zenzaiInferLimit,
                         int numberFullwidth, int symbolFullwidth,
                         int periodStyleIndex, int commaStyleIndex,
                         int spaceFullwidth, int tenCombining,
                         std::string profileText);

    void newComposingText();

    void completePrefix(int index);

    struct CandidateData {
        std::string candidateText;
        std::string subHiragana;
        bool liveCompat;
    };

    std::vector<CandidateData> getCandidates(bool isPredictMode, int n_best);

   private:
    bool retry_connect();
    void start_hazkey_server();
    bool requestSuccess(hazkey::ResponseEnvelope);
    int sock_ = -1;
    std::string socket_path_;
};

#endif  // HAZKEY_SERVER_CONNECTOR_H

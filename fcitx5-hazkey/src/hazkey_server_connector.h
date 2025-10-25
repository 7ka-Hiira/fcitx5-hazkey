#ifndef HAZKEY_SERVER_CONNECTOR_H
#define HAZKEY_SERVER_CONNECTOR_H

#include <fcitx-utils/log.h>
#include <fcitx/text.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <string>

#include "base.pb.h"
#include "commands.pb.h"
#include "env_config.h"

class HazkeyServerConnector {
   public:
    // HazkeyServerConnector();
    // ~HazkeyServerConnector();

    HazkeyServerConnector() {
        kill_existing_hazkey_server();
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

    fcitx::Text getComposingHiraganaWithCursor();

    void inputChar(std::string text);

    void shiftKeyEvent(bool isRelease);

    bool currentInputModeIsDirect();

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
    };

    hazkey::commands::CandidatesResult getCandidates(bool isSuggest);

   private:
    bool retry_connect();
    void start_hazkey_server();
    void kill_existing_hazkey_server();
    bool is_hazkey_server_running();
    bool requestSuccess(hazkey::ResponseEnvelope);
    int sock_ = -1;
    std::string socket_path_;
};

#endif  // HAZKEY_SERVER_CONNECTOR_H

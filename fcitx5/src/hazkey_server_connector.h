#include <fcitx-utils/log.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#include "env_config.h"

using json = nlohmann::json;

std::string get_socket_path();

int connect_server();

json transact(int sock, const json& send_data);

std::string getComposingText(int sock, std::string type);

std::string getComposingHiraganaWithCursor(int sock);

void addToComposingText(int sock, std::string text, bool isDirect);

void deleteLeft(int sock);

void deleteRight(int sock);

void moveCursor(int sock, int offset);

void setLeftContext(int sock, std::string context, int anchor);

void setServerConfig(int sock, int zenzaiEnabled, int zenzaiInferLimit,
                     int numberFullwidth, int symbolFullwidth,
                     int periodStyleIndex, int commaStyleIndex,
                     int spaceFullwidth, int tenCombining,
                     std::string profileText);

void createComposingTextInstance(int sock);

void completePrefix(int sock);

std::vector<std::string> getServerCandidates(int sock, bool isPredictMode,
                                             int n_best);

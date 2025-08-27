#ifndef SERVERCONNECTOR_H
#define SERVERCONNECTOR_H

#include <optional>
#include <string>

#include "protocol/base.pb.h"

class ServerConnector {
   public:
    ServerConnector();
    void connect_server();
    std::optional<hazkey::config::CurrentConfig> getConfig();
    void setCurrentConfig(hazkey::config::CurrentConfig);

   private:
    std::string get_socket_path();
    void restart_hazkey_server();
    std::optional<hazkey::ResponseEnvelope> transact(
        const hazkey::RequestEnvelope& send_data);

    int sock_ = -1;
    std::string socket_path_;
};

#endif  // SERVERCONNECTOR_H

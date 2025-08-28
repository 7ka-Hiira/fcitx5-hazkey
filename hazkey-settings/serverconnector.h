#ifndef SERVERCONNECTOR_H
#define SERVERCONNECTOR_H

#include <optional>
#include <string>

#include "base.pb.h"

class ServerConnector {
   public:
    ServerConnector();
    std::optional<hazkey::config::CurrentConfig> getConfig();
    void setCurrentConfig(hazkey::config::CurrentConfig);

   private:
    std::string get_socket_path();
    void restart_hazkey_server();
    int create_connection();
    std::optional<hazkey::ResponseEnvelope> transact(
        const hazkey::RequestEnvelope& send_data);
};

#endif  // SERVERCONNECTOR_H

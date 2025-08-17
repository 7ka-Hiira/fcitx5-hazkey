#ifndef SERVERCONNECTOR_H
#define SERVERCONNECTOR_H

#include <string>
#include <optional>
#include "protocol/base.pb.h"

class ServerConnector {
   public:
    ServerConnector();
    void connect_server();
    std::optional<hazkey::config::CurrentConfig> getCurrentConfig();

   private:
    std::string get_socket_path();
    void start_hazkey_server();
    std::optional<hazkey::ResponseEnvelope> transact(const hazkey::RequestEnvelope& send_data);

    int sock_ = -1;
    std::string socket_path_;
};

#endif  // SERVERCONNECTOR_H

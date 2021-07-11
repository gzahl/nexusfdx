#include "TcpServer.h"

TcpServer::TcpServer(uint16_t port) {
  asyncServer = new AsyncServer(port);
  debugI("Started TCP Server on port: %u", port);

  asyncServer->onClient(
      [](void *arg1, AsyncClient *client) {
        std::unordered_set<AsyncClient *> *asyncClients_onClient =
            static_cast<std::unordered_set<AsyncClient *> *>(arg1);

        asyncClients_onClient->insert(client);

        client->onDisconnect(
            [](void *arg2, AsyncClient *disconnectingClient) {
              std::unordered_set<AsyncClient *> *asyncClients_onDisconnect =
                  static_cast<std::unordered_set<AsyncClient *> *>(arg2);
              Serial.printf(
                  "Client has disconnected from port %u, ip: %s, No of "
                  "clients: "
                  "%u\n",
                  disconnectingClient->localPort(),
                  disconnectingClient->remoteIP().toString().c_str(),
                  asyncClients_onDisconnect->size() - 1);
              asyncClients_onDisconnect->erase(disconnectingClient);
            },
            arg1);

        Serial.printf(
            "New client has been connected to server on port %u, ip: %s, No of "
            "clients: %u\n",
            client->localPort(), client->remoteIP().toString().c_str(),
            asyncClients_onClient->size());

        /*
        // register events
        client->onData(&handleData, NULL);
        client->onError(&handleError, NULL);
        client->onDisconnect(&handleDisconnect, NULL);
        client->onTimeout(&handleTimeOut, NULL);*/
      },
      static_cast<void *>(&asyncClients));
  asyncServer->begin();
}

void TcpServer::send(const char *data) {
  for (AsyncClient *client : asyncClients) {
    client->write(data);
  }
}

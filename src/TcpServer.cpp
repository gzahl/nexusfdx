#include "TcpServer.h"

using sensesp::Debug;

TcpServer::TcpServer(uint16_t port) {
  asyncServer = new AsyncServer(port);
  debugI("Started TCP Server on port: %u", port);

  asyncServer->onClient(
      [this](void *arg1, AsyncClient *client) {
        this->asyncClients.insert(client);

        client->onDisconnect(
            [this](void *arg2, AsyncClient *disconnectingClient) {
              this->asyncClients.erase(disconnectingClient);
              debugI(
                  "Client has disconnected from port %u, ip: %s, No of "
                  "clients: "
                  "%u",
                  disconnectingClient->localPort(),
                  disconnectingClient->remoteIP().toString().c_str(),
                  this->asyncClients.size());
            },
            arg1);

        debugI(
            "New client has been connected to server on port %u, ip: %s, No of "
            "clients: %u",
            client->localPort(), client->remoteIP().toString().c_str(),
            this->asyncClients.size());
        client->onError(
            [this](void *arg2, AsyncClient *client, int8_t error) {
              debugE("connection error %s from client %s",
                     client->errorToString(error),
                     client->remoteIP().toString().c_str());
              client->close();
            },
            arg1);
        client->onTimeout(
            [this](void *arg2, AsyncClient *client, uint32_t time) {
              debugE("client ACK timeout ip: %s",
                     client->remoteIP().toString().c_str());
              client->close();
            },
            NULL);
      },
      NULL);
  asyncServer->begin();
}

void TcpServer::send(const char *data) {
  for (AsyncClient *client : asyncClients) {
    client->write(data);
  }
}

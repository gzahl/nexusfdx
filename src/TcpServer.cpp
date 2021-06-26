#include "TcpServer.h"

std::unordered_set<AsyncClient *> TcpServer::asyncClients;

TcpServer::TcpServer(uint16_t port) {
  asyncServer = new AsyncServer(port);
  Serial.printf("Started TCP Server on port: %u\n", port);

  asyncServer->onClient(
      [](void *arg, AsyncClient *client) {
        TcpServer::asyncClients.insert(client);
        client->onDisconnect([](void *arg2, AsyncClient *disconnectingClient) {
          TcpServer::asyncClients.erase(disconnectingClient);
        });

        Serial.printf("new client has been connected to server, ip: %s\n",
                      client->remoteIP().toString().c_str());
        /*
        // register events
        client->onData(&handleData, NULL);
        client->onError(&handleError, NULL);
        client->onDisconnect(&handleDisconnect, NULL);
        client->onTimeout(&handleTimeOut, NULL);*/
      },
      asyncServer);
  asyncServer->begin();
}

void TcpServer::send(const char *data) {
  for (AsyncClient *client : TcpServer::asyncClients) {
    client->write(data);
  }
}

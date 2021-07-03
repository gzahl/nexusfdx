#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <Arduino.h>
#include <AsyncTCP.h>

#include <unordered_set>

#include "NetworkPublisher.h"

class TcpServer : public NetworkPublisher {
 public:
  TcpServer(uint16_t port);
  void send(const char* data);

 private:
  AsyncServer* asyncServer;
  std::unordered_set<AsyncClient *> asyncClients;
};

#endif
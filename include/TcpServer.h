#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <AsyncTCP.h>

#include <unordered_set>

#include "FreeRTOS.h"
#include "NetworkPublisher.h"

class TcpServer : NetworkPublisher {
 public:
  TcpServer(uint16_t port);
  void send(const char* data);

 private:
  AsyncServer* asyncServer;
  static std::unordered_set<AsyncClient*> asyncClients;
};

#endif
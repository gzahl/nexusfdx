#ifndef UDPSERVER_H
#define UDPSERVER_H

#include <stdint.h>

#include "AsyncUDP.h"
#include "NetworkPublisher.h"

class UdpServer : NetworkPublisher {
 public:
  UdpServer(uint16_t port);
  void send(const char* data);

 private:
  AsyncUDP udp;
  uint16_t broadcastPort;
};

#endif
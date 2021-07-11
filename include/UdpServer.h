#ifndef UDPSERVER_H
#define UDPSERVER_H

#include <Arduino.h>
#include "AsyncUDP.h"
#include "NetworkPublisher.h"
#include "sensesp.h"

class UdpServer : public NetworkPublisher {
 public:
  UdpServer(uint16_t port);
  void send(const char* data);

 private:
  AsyncUDP udp;
  uint16_t broadcastPort;
};

#endif
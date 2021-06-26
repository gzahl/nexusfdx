#include "UdpServer.h"

UdpServer::UdpServer(uint16_t port) { broadcastPort = port; }

void UdpServer::send(const char* data) { udp.broadcastTo(data, broadcastPort); }
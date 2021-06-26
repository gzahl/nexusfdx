#ifndef NETWORKPUBLISHER_H
#define NETWORKPUBLISHER_H

class NetworkPublisher {
 public:
  virtual void send(const char* data);
};

#endif
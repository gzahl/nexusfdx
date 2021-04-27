#ifndef SERIALFDXLISTENERTASK_H
#define SERIALFDXLISTENERTASK_H

#include "SerialListenerTask.h"
#include "MovingAverage.h"


class SerialFdxListenerTask : public SerialListenerTask {
public:
  SerialFdxListenerTask(
      uint32_t baud, SoftwareSerialConfig config, int8_t rxPin, int8_t txPin,
      std::function<void(std::vector<uint8_t> &)> sentenceCallback_);

private:
  std::vector<uint8_t> msg;
  void TaskLoop() override;
  bool readNmea(uint8_t byte);
  unsigned char reverse(unsigned char);
  void printMessage(unsigned char *msg, unsigned char len);
  void readMessage(unsigned char *msg, unsigned char len);
  void readData(unsigned char messageId, unsigned char *msg, unsigned char len);
  void readMsg18(uint8_t *payload);
  void readMsg21(uint8_t *payload);
  void readMsg112(uint8_t *payload);
  unsigned char calcChksum(unsigned char *msg, unsigned char len);

  unsigned char len;
  unsigned char byte;
  unsigned char message[50];
  // MovingAverage <float>vavg(5,0.0);
};

#endif
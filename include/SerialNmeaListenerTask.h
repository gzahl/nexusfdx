#ifndef SERIALNMEALISTENERTASK_H
#define SERIALNMEALISTENERTASK_H

#include "SerialListenerTask.h"

class SerialNmeaListenerTask : public SerialListenerTask {
public:
  SerialNmeaListenerTask(
      uint32_t baud, SoftwareSerialConfig config, int8_t rxPin, int8_t txPin,
      std::function<void(std::vector<uint8_t> &)> sentenceCallback_);

private:
  std::vector<uint8_t> msg;
  void TaskLoop() override;
  bool readNmea(uint8_t byte);
};

#endif
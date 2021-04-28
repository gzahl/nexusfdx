#ifndef SERIALNMEALISTENERTASK_H
#define SERIALNMEALISTENERTASK_H

#include "SerialListenerTask.h"

class SerialNmeaListenerTask : public SerialListenerTask {
public:
  SerialNmeaListenerTask(
      const char *name, uint32_t baud, SoftwareSerialConfig config,
      int8_t rxPin, int8_t txPin,
      std::function<void(std::vector<uint8_t> &)> sentenceCallback_);
  void TaskLoop();

private:
  std::vector<uint8_t> msg;
  TaskHandle_t *moduleLoopTaskHandle;
  std::function<void(std::vector<uint8_t> &)> sentenceCallback;
  static void TaskStart(void *thisPointer);
  bool readNmea(uint8_t byte);
};

#endif
#ifndef SERIALNMEALISTENERTASK_H
#define SERIALNMEALISTENERTASK_H

#include "SerialListenerTask.h"

class SerialNmeaListenerTask : public SerialListenerTask {
public:
  SerialNmeaListenerTask(
      const char *name, SoftwareSerial *swSerial,
      std::function<void(std::vector<uint8_t> &)> sentenceCallback_);
  void TaskLoop();

private:
  std::vector<uint8_t> msg;
  TaskHandle_t moduleLoopTaskHandle = NULL;
  std::function<void(std::vector<uint8_t> &)> sentenceCallback;
  static void TaskStart(void *thisPointer);
  bool readNmea(uint8_t byte);
  SoftwareSerial *swSerial;
};

#endif
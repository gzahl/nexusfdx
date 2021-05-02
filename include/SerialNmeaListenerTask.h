#ifndef SERIALNMEALISTENERTASK_H
#define SERIALNMEALISTENERTASK_H

#include "SerialListenerTask.h"

class SerialNmeaListenerTask : public SerialListenerTask {
public:
  SerialNmeaListenerTask(
      const char *name, SoftwareSerial *swSerial,
      std::function<void(std::vector<uint8_t> &)> sentenceCallback_, bool debug_=false);
  void TaskLoop();

private:
  std::vector<uint8_t> msg;
  bool readNmea(uint8_t byte);
  TaskHandle_t moduleLoopTaskHandle = NULL;
  std::function<void(std::vector<uint8_t> &)> sentenceCallback;
  static void TaskStart(void *thisPointer);
  SoftwareSerial *swSerial;
  bool debug;
};

#endif
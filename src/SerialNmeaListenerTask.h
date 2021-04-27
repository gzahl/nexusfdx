#ifndef SERIALNMEALISTENERTASK_H
#define SERIALNMEALISTENERTASK_H

#include <Arduino.h>
#include <SoftwareSerial.h>

class SerialNmeaListenerTask {
public:
  SerialNmeaListenerTask(
      uint32_t baud, SoftwareSerialConfig config, int8_t rxPin, int8_t txPin,
      std::function<void(std::vector<uint8_t> &)> sentenceCallback_);

private:
  TaskHandle_t *moduleLoopTaskHandle;
  static void TaskStart(void *taskStartParameters);
  void TaskLoop();
  bool readNmea(uint8_t byte);
  std::vector<uint8_t> msg;
  SoftwareSerial swSerial;
  std::function<void(std::vector<uint8_t> &)> sentenceCallback;
};

#endif
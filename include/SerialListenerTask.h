#ifndef SERIALLISTENERTASK_H
#define SERIALLISTENERTASK_H

#include <Arduino.h>
#include <SoftwareSerial.h>

class SerialListenerTask {
public:
  SerialListenerTask(
      uint32_t baud, SoftwareSerialConfig config, int8_t rxPin, int8_t txPin,
      std::function<void(std::vector<uint8_t> &)> sentenceCallback_);

protected:
  virtual void TaskLoop() = 0;
  SoftwareSerial swSerial;
  std::function<void(std::vector<uint8_t> &)> sentenceCallback;

private:
  TaskHandle_t *moduleLoopTaskHandle;
  static void TaskStart(void *taskStartParameters);
};

#endif
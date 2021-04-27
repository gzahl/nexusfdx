#include "SerialListenerTask.h"

SerialListenerTask::SerialListenerTask(
    uint32_t baud, SoftwareSerialConfig config, int8_t rxPin, int8_t txPin,
    std::function<void(std::vector<uint8_t> &)> sentenceCallback_) {
  swSerial.begin(baud, config, rxPin, txPin);
  sentenceCallback = sentenceCallback_;
  xTaskCreate(SerialListenerTask::TaskStart, "NAME", 2048, this, tskNO_AFFINITY,
              moduleLoopTaskHandle);
}

void SerialListenerTask::TaskStart(void *taskStartParameters) {
  SerialListenerTask *moduleObject = static_cast<SerialListenerTask *>(taskStartParameters);
  moduleObject->TaskLoop();
}
#include "SerialNmeaListenerTask.h"

SerialNmeaListenerTask::SerialNmeaListenerTask(
    const char *name, uint32_t baud, SoftwareSerialConfig config, int8_t rxPin,
    int8_t txPin, std::function<void(std::vector<uint8_t> &)> sentenceCallback_)
    : SerialListenerTask(baud, config, rxPin, txPin) {
  msg.reserve(80);
  sentenceCallback = sentenceCallback_;
  xTaskCreate(SerialNmeaListenerTask::TaskStart, name, 2048, this,
              tskNO_AFFINITY, moduleLoopTaskHandle);
}

void SerialNmeaListenerTask::TaskStart(void *thisPointer) {
  static_cast<SerialNmeaListenerTask *>(thisPointer)->TaskLoop();
}

void SerialNmeaListenerTask::TaskLoop() {
  while (true) {
    while (swSerial.available()) {
      if (readNmea(swSerial.read()))
        sentenceCallback(msg);
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
  vTaskDelete(NULL);
}

bool SerialNmeaListenerTask::readNmea(uint8_t byte) {
  if (byte == '$') {
    msg.clear();
  }
  msg.push_back(byte);
  if (byte == '\n') {
    msg.push_back('\0');
    return true;
  }
  return false;
}
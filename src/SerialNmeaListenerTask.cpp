#include "SerialNmeaListenerTask.h"

SerialNmeaListenerTask::SerialNmeaListenerTask(
    const char *name, SoftwareSerial *swSerial_,
    std::function<void(std::vector<uint8_t> &)> sentenceCallback_) {
  msg.reserve(80);
  swSerial = swSerial_;
  sentenceCallback = sentenceCallback_;
  xTaskCreate(SerialNmeaListenerTask::TaskStart, name, 2048, this,
              tskNO_AFFINITY, moduleLoopTaskHandle);
}

void SerialNmeaListenerTask::TaskStart(void *thisPointer) {
  static_cast<SerialNmeaListenerTask *>(thisPointer)->TaskLoop();
}

void SerialNmeaListenerTask::TaskLoop() {
  while (true) {
    while (swSerial->available()) {
      if (readNmea(swSerial->read()))
        sentenceCallback(msg);
    }
    vTaskDelay(pdMS_TO_TICKS(10));
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
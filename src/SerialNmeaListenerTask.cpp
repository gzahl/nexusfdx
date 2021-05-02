#include "SerialNmeaListenerTask.h"

SerialNmeaListenerTask::SerialNmeaListenerTask(
    const char *name, SoftwareSerial *swSerial_,
    std::function<void(std::vector<uint8_t> &)> sentenceCallback_,
    bool debug_) {
  msg.reserve(80);
  swSerial = swSerial_;
  sentenceCallback = sentenceCallback_;
  debug = debug_;
  xTaskCreate(SerialNmeaListenerTask::TaskStart, name, 2048, (void *)this,
              tskNO_AFFINITY, &moduleLoopTaskHandle);
}

void SerialNmeaListenerTask::TaskStart(void *thisPointer) {
  static_cast<SerialNmeaListenerTask *>(thisPointer)->TaskLoop();
}

void SerialNmeaListenerTask::TaskLoop() {
  SoftwareSerial sw;
  sw.begin(9600, SWSERIAL_8N1, 23, 19);
  while(true) {
    while(sw.available()) {
      char c = sw.read();
      Serial.print(c);
    }
    vTaskDelay(1);
  }
  while (true) {
    while (swSerial->available()) {
      if (readNmea(swSerial->read())) {
        sentenceCallback(msg);
      }
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
  vTaskDelete(NULL);
}

bool SerialNmeaListenerTask::readNmea(uint8_t byte) {
  if (byte == '$' || msg.size() > 80) {
    msg.clear();
  }
  msg.push_back(byte);
  if (debug) {
    Serial.print(byte);
  }
  if (byte == '\n') {
    msg.push_back('\0');
    return true;
  }
  return false;
}
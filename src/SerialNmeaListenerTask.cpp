#include "SerialNmeaListenerTask.h"

SerialNmeaListenerTask::SerialNmeaListenerTask(
    uint32_t baud, SoftwareSerialConfig config, int8_t rxPin, int8_t txPin,
    std::function<void(std::vector<uint8_t> &)> sentenceCallback_)
    : SerialListenerTask(baud, config, rxPin, txPin, sentenceCallback_) {
  msg.reserve(80);
}

void SerialNmeaListenerTask::TaskLoop() {
  while (true) {
    if (swSerial.available() && readNmea(swSerial.read())) {
      sentenceCallback(msg);
    }
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
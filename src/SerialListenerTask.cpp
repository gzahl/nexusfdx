#include "SerialListenerTask.h"

SerialListenerTask::SerialListenerTask(uint32_t baud,
                                       SoftwareSerialConfig config,
                                       int8_t rxPin, int8_t txPin) {

  swSerial.begin(baud, config, rxPin, txPin);
}

SoftwareSerial &SerialListenerTask::getSoftwareSerial() { return swSerial; }
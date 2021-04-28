#ifndef SERIALLISTENERTASK_H
#define SERIALLISTENERTASK_H

#include <Arduino.h>
#include <SoftwareSerial.h>

class SerialListenerTask {
public:
  SerialListenerTask(uint32_t baud, SoftwareSerialConfig config, int8_t rxPin,
                     int8_t txPin);
  SoftwareSerial &getSoftwareSerial();

protected:
  SoftwareSerial swSerial;
};

#endif
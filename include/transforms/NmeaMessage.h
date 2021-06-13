#ifndef _NmeaMessage_H_
#define _NmeaMessage_H_

#include <ctime>

#include "FdxSource.h"
#include "system/valueconsumer.h"
#include "transforms/transform.h"

typedef enum {
  TRUE_WIND,
  APPARENT_WIND,
  WATER_TEMPERATURE,
  VOLTAGE
} MessageType;

/**
 * @brief Consumes an object and produces a nmea message.
 */
class NmeaMessage : public Transform<float, String> {
 public:
  NmeaMessage(MessageType messageType, String config_path = "");
  virtual void set_input(float input, uint8_t input_channel = 0) override;

 private:
  MessageType messageType;
  uint8_t received = 0;
  float inputs[8];
  String calcChecksum(char *nmea_data);
  String writeSentenceMWV(char trueOrApparent, float direction,
                          float windspeed);
  String writeSentenceMTW(float waterTemperature);
};

#endif
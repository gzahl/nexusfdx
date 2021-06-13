#ifndef _NmeaMessage_H_
#define _NmeaMessage_H_

#include <ctime>

#include "FdxSource.h"
#include "system/valueconsumer.h"
#include "transforms/transform.h"

/**
 * @brief Consumes an object and produces a nmea message.
 */
class NmeaMessage : public Transform<float, String> {
 public:
  NmeaMessage(char relativeOrTrue = 'T', String config_path = "");
  virtual void set_input(float input, uint8_t input_channel = 0) override;

 private:
  char relativeOrTrue_;
  uint8_t received = 0;
  float inputs[2];
  String calcChecksum(char *nmea_data);
  String writeSentenceMWV(char trueOrApparent, float direction,
                          float windspeed);
};

#endif
#ifndef _NmeaMessage_H_
#define _NmeaMessage_H_

#include <ctime>

#include "FdxSource.h"
#include "system/valueconsumer.h"
#include "transforms/transform.h"

typedef enum {
  NMEA_MWV_TRUE,
  NMEA_MWV_RELATIVE,
  NMEA_MTW,
  NMEA_XDR_VOLTAGE,
  NMEA_XDR_SIGNALSTRENGTH
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
  String writeSentenceXDR(char type, char unit, const char* name, float value);
};

#endif
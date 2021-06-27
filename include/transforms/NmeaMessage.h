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
  NMEA_XDR_SIGNALSTRENGTH,
  NMEA_XDR_PITCH,
  NMEA_XDR_ROLL,
  NMEA_DPT,
  NMEA_HDM
} MessageType;

/**
 * @brief Consumes an object and produces a nmea message.
 */
template<typename T>
class NmeaMessage : public Transform<T, String> {
 public:
  NmeaMessage(MessageType messageType, String config_path = "");
  virtual void set_input(T input, uint8_t input_channel = 0) override;

 private:
  MessageType messageType;
  uint8_t received = 0;
  T inputs[8];
  char deviceId[3];
  String calcChecksum(char *nmea_data);
  String writeSentenceMWV(char trueOrApparent, T direction,
                          T windspeed);
  String writeSentenceMTW(T waterTemperature);
  String writeSentenceXDR(char type, char unit, const char* name, T value);
  String writeSentenceDPT(T depth, T offset);
  String writeSentenceHDM(T heading);
};

#endif
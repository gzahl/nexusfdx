#ifndef FDXPARSER_H
#define FDXPARSER_H

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

typedef enum {
  UNKNOWN = 256,
  ERROR = 257,
  WRONG_CHKSUM = 258,
  APPARENT_WIND = 18,
  WIND_TRANSDUCER_SIGNAL = 112,
  TRUE_WIND = 1,
  TEMPERATURE = 8,
  VOLTAGE = 9,
  DEPTH = 7
} FdxType;

typedef struct {
  float angle;
  float speed;
} WindType;

struct FdxData {
  FdxType type;
  WindType apparantWind;
  WindType trueWind;
  float signalStrength;
  float temperature;
  float voltage;
  float depth;
};

class FdxParser {
 public:
  FdxParser();
  void parse(unsigned char *msg, unsigned char len);
  FdxData data;

 private:
  unsigned char reverse(unsigned char);
  // void printMessage(unsigned char *msg, unsigned char len);
  void readData(unsigned char messageId, unsigned char *msg, unsigned char len);
  void readWind(uint8_t *payload, float &angle, float &speed);
  void readMsg21(uint8_t *payload);
  void readMsg112(uint8_t *payload);
  unsigned char calcChksum(unsigned char *msg, unsigned char len);
};

#endif
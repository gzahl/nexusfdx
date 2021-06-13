#ifndef FDXPARSER_H
#define FDXPARSER_H

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

typedef enum {
  UNKNOWN = 256,
  ERROR = 257,
  WRONG_CHKSUM = 258,
  WIND = 18,
  WIND_TRANSDUCER_SIGNAL = 112,
  OTHER_WIND = 1
} FdxType;

typedef struct {
  float angle;
  float speed;
} WindType;

struct FdxData {
  FdxType type;
  WindType relativeWind;
  WindType otherWind;
  float signalStrength;
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
  void readWind(uint8_t *payload, float& angle, float& speed);
  void readMsg21(uint8_t *payload);
  void readMsg112(uint8_t *payload);
  unsigned char calcChksum(unsigned char *msg, unsigned char len);
};

#endif
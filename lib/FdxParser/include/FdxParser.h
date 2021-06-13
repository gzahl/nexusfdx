#ifndef FDXPARSER_H
#define FDXPARSER_H

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

typedef enum {
  UNKNOWN = 256,
  ERROR = 257,
  WRONG_CHKSUM = 258,
  WIND = 18
} FdxType;

struct FdxData {
  FdxType type;
  struct {
    float angle;
    float speed;
  } relativeWind;
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
  void readMsg18(uint8_t *payload);
  void readMsg21(uint8_t *payload);
  void readMsg112(uint8_t *payload);
  unsigned char calcChksum(unsigned char *msg, unsigned char len);
};

#endif
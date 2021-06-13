#ifndef FDXPARSER_H
#define FDXPARSER_H

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

typedef enum { UNKNOWN, WIND } FdxType;

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
  //void printMessage(unsigned char *msg, unsigned char len);
  void readData(unsigned char messageId, unsigned char *msg, unsigned char len);
  void readMsg18(uint8_t *payload);
  void readMsg21(uint8_t *payload);
  void readMsg112(uint8_t *payload);
  unsigned char calcChksum(unsigned char *msg, unsigned char len);

  unsigned char len;
  unsigned char byte;
  unsigned char message[50];
};

#endif
#ifndef FDXPARSER_H
#define FDXPARSER_H

#include "system/observablevalue.h"

struct Wind {
  ObservableValue<float> direction;
  ObservableValue<float> speed;
};

struct FdxData {
  Wind relativeWind;
  ObservableValue<String> rawMessage;

};

class FdxParser {
public:
  FdxParser(FdxData* fdxData);
  void parse(unsigned char *msg, unsigned char len);

private:
  FdxData* fdxData;
  String msg;
  String chksum;

  unsigned char reverse(unsigned char);
  void printMessage(unsigned char *msg, unsigned char len);
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
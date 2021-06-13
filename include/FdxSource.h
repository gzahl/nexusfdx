#ifndef FDXSOURCE_H
#define FDXSOURCE_H

#include <SoftwareSerial.h>

#include "sensors/sensor.h"
#include <FdxParser.h>

class FdxSource : public Sensor {
public:
  FdxSource(SoftwareSerial *rx_stream);
  virtual void enable() override final;
  FdxData fdxData;

private:
  FdxParser fdxParser;
  SoftwareSerial *rx_stream_;
  String msg;
  String chksum;

  unsigned char reverse(unsigned char);
  void printMessage(unsigned char *msg, unsigned char len);
  void emitRawMessage(unsigned char *msg, unsigned char len);
  void readData(unsigned char messageId, unsigned char *msg, unsigned char len);

  unsigned char len;
  unsigned char byte;
  unsigned char message[50];
};

#endif
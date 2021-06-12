#ifndef FDXSOURCE_H
#define FDXSOURCE_H

#include <SoftwareSerial.h>

#include "sensors/sensor.h"
#include "system/observablevalue.h"

struct Wind {
  ObservableValue<float> direction;
  ObservableValue<float> speed;
};

struct FdxData {
  Wind apparentWind;
  Wind trueWind;
  ObservableValue<String> rawMessage;

};

class FdxSource : public Sensor {
public:
  FdxSource(SoftwareSerial *rx_stream);
  virtual void enable() override final;
  FdxData fdxData;

private:
  SoftwareSerial *rx_stream_;
  String msg;
  String chksum;

  unsigned char reverse(unsigned char);
  void printMessage(unsigned char *msg, unsigned char len);
  void emitRawMessage(unsigned char *msg, unsigned char len);
  void readMessage(unsigned char *msg, unsigned char len);
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
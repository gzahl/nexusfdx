#ifndef FDXSOURCE_H
#define FDXSOURCE_H

#include <FdxParser.h>
#include <SoftwareSerial.h>

#include "sensors/sensor.h"
#include "system/observablevalue.h"

typedef struct {
  ObservableValue<float> angle;
  ObservableValue<float> speed;
} ObsWindType;

struct Data {
  ObsWindType apparantWind;
  ObsWindType trueWind;
  ObservableValue<float> temperature;
  ObservableValue<float> voltage;
  ObservableValue<float> signalStrength;
  ObservableValue<float> depth;
  ObservableValue<String> rawMessage;
};

class FdxSource : public Sensor {
 public:
  FdxSource(SoftwareSerial *rx_stream);
  virtual void enable() override final;
  Data data;

 private:
  FdxParser fdxParser;
  SoftwareSerial *rx_stream_;
  String msg;
  String chksum;

  unsigned char reverse(unsigned char);
  void printMessage(unsigned char *msg, unsigned char len);
  void readMessage(unsigned char *msg, unsigned char len);
  void emitRawMessage(unsigned char *msg, unsigned char len);
  void readData(unsigned char messageId, unsigned char *msg, unsigned char len);

  unsigned char len;
  unsigned char byte;
  unsigned char message[50];
};

#endif
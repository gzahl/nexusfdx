#ifndef FDXSOURCE_H
#define FDXSOURCE_H

#include <FdxParser.h>
#include <SoftwareSerial.h>

#include "sensesp/sensors/sensor.h"
#include "sensesp/system/observablevalue.h"

typedef struct {
  sensesp::ObservableValue<float> angle;
  sensesp::ObservableValue<float> speed;
} ObsWindType;

struct Data {
  ObsWindType apparantWind;
  ObsWindType trueWind;
  sensesp::ObservableValue<float> temperature;
  sensesp::ObservableValue<float> voltage;
  sensesp::ObservableValue<float> signalStrength;
  sensesp::ObservableValue<float> depth;
  sensesp::ObservableValue<String> rawMessage;
};

class FdxSource : public sensesp::Sensor {
 public:
  FdxSource(SoftwareSerial *rx_stream);
  virtual void start() override final;
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
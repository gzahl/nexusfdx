#ifndef _NmeaSentenceParser_H_
#define _NmeaSentenceParser_H_

#include "sensors/sensor.h"
#include "system/observablevalue.h"

class NmeaSentenceParser : public Sensor {
 public:
  NmeaSentenceParser(Stream* rx_stream);
  virtual void enable() override final;
  ObservableValue<String> nmeaSentence;
 private:
  Stream* rx_stream_;
  String msg;
  bool readNmea(uint8_t byte);
 
};

#endif
#ifndef _NmeaSentenceSource_H_
#define _NmeaSentenceSource_H_

#include "sensors/sensor.h"
#include "system/observablevalue.h"

class NmeaSentenceSource : public Sensor {
 public:
  NmeaSentenceSource(Stream *rx_stream);
  virtual void enable() override final;
  ObservableValue<String> nmeaSentence;

 private:
  Stream *rx_stream_;
  String msg;
  String chksum;
  void handle(char c);

  void (NmeaSentenceSource::*current_state)(char);
  void state_start(char c);
  void state_in_term(char c);
  void state_in_checksum(char c);
  int parity;
  bool validate_checksum();
};

#endif
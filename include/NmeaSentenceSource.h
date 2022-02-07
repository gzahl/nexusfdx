#ifndef _NmeaSentenceSource_H_
#define _NmeaSentenceSource_H_

#include "sensesp/sensors/sensor.h"
#include "sensesp/system/observable.h"

class NmeaSentenceSource : public sensesp::Sensor, public sensesp::ValueProducer<String>, public sensesp::ValueConsumer<String> {
 public:
  NmeaSentenceSource(Stream *rx_stream, const char* name);
  virtual void start() override final;
  void set_input(String new_value, uint8_t input_channel);

 private:
  Stream *rx_stream_;
  String msg;
  String chksum;
  void handle(char c);
  String name_;

  void (NmeaSentenceSource::*current_state)(char);
  void state_start(char c);
  void state_in_term(char c);
  void state_in_checksum(char c);
  int parity;
  bool validate_checksum();
};

#endif
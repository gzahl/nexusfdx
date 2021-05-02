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

    void (NmeaSentenceParser::*current_state)(char);
  void state_start(char c);
  void state_in_term(char c);
  void state_in_checksum(char c);
  // pointer for the next character in buffer
  int cur_offset;
  // pointer for the current term in buffer
  int cur_term;
  int parity;
  bool validate_checksum();

 
};

#endif
#include "NmeaSentenceSource.h"

#include <math.h>

#include "sensesp.h"

NmeaSentenceSource::NmeaSentenceSource(Stream *rx_stream, const char* name) : Sensor(), ValueProducer(), ValueConsumer(), name_(name) {
  rx_stream_ = rx_stream;
  current_state = &NmeaSentenceSource::state_start;
}

void NmeaSentenceSource::enable() {
  // enable reading the serial port
  debugI("Enabling NmeaSentenceSource!");
  app.onAvailable(*rx_stream_, [this]() {
    while (rx_stream_->available()) {
      char c = rx_stream_->read();
      if (false) {
        printf("%c", c);
      }
      handle(c);
    }
  });
}

void NmeaSentenceSource::set_input(String new_value, uint8_t input_channel) {
  rx_stream_->write(new_value.c_str());
}


void NmeaSentenceSource::handle(char c) { (this->*(current_state))(c); }

void NmeaSentenceSource::state_start(char c) {
  switch (c) {
    case '$':
    case '!':
      current_state = &NmeaSentenceSource::state_in_term;
      msg = c;
      parity = 0;
      break;
    default:
      // anything else can be ignored
      break;
  }
}

void NmeaSentenceSource::state_in_term(char c) {
  switch (c) {
    case '\r':
    case '\n':
      // end of sentence before checksum has been read
      current_state = &NmeaSentenceSource::state_start;
      break;
    case '*':
      current_state = &NmeaSentenceSource::state_in_checksum;
      msg.concat(c);
      chksum.clear();
      break;
    default:
      if (msg.length() > 80) {
        debugW("Message too long for NmeaSource '%s', reset.\n", name_);
        current_state = &NmeaSentenceSource::state_start;
      } else {
        msg.concat(c);
        parity ^= c;
      }
      break;
  }
}

void NmeaSentenceSource::state_in_checksum(char c) {
  switch (c) {
    case ',':
    case '*':
      // there shouldn't be new terms after the checksum
      current_state = &NmeaSentenceSource::state_start;
    case '\r':
    case '\n':
      // end of sentence
      msg.concat(chksum);
      msg.concat("\r\n");
      if (!validate_checksum()) {
        debugW("Chksum wrong for NmeaSource '%s'! '%s'\n", name_.c_str(), msg.c_str());
        current_state = &NmeaSentenceSource::state_start;
        return;
      }
      emit(msg);
      current_state = &NmeaSentenceSource::state_start;
      break;
    default:
      chksum.concat(c);
      break;
  }
}

bool NmeaSentenceSource::validate_checksum() {
  int checksum;
  sscanf(chksum.c_str(), "%2x", &checksum);
  return this->parity == checksum;
}

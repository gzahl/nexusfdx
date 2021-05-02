#include "NmeaSentenceSource.h"

#include <math.h>

#include "sensesp.h"

NmeaSentenceSource::NmeaSentenceSource(Stream *rx_stream) : Sensor() {
  rx_stream_ = rx_stream;
  current_state = &NmeaSentenceSource::state_start;
}

void NmeaSentenceSource::enable() {
  // enable reading the serial port
  Serial.println("Enabling NmeaSentencerParser!");
  app.onAvailable(*rx_stream_, [this]() {
    while (rx_stream_->available()) {
      handle(rx_stream_->read());
    }
  });
}

void NmeaSentenceSource::handle(char c) { (this->*(current_state))(c); }

void NmeaSentenceSource::state_start(char c) {
  switch (c) {
  case '$':
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
    chksum.clear();
  default:
    if (msg.length() > 80) {
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
    if (!validate_checksum()) {
      current_state = &NmeaSentenceSource::state_start;
      return;
    }
    nmeaSentence.emit(msg);
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

#include "NmeaSentenceParser.h"

#include <math.h>

#include "sensesp.h"

NmeaSentenceParser::NmeaSentenceParser(Stream* rx_stream)
    : Sensor() {
  rx_stream_ = rx_stream;

}

void NmeaSentenceParser::enable() {
  // enable reading the serial port
  Serial.println("Enabling NmeaSentencerParser!");
  app.onAvailable(*rx_stream_, [this]() {
    while (rx_stream_->available()) {
      readNmea(rx_stream_->read());
    }
  });
}

bool NmeaSentenceParser::readNmea(uint8_t byte) {
  //Serial.print((char) byte);
  if (byte == '$' || msg.length() > 80) {
    msg.clear();
  }
  if (byte == '\n') {
    nmeaSentence.emit(msg);
    return true;
  } else {
    msg.concat((char) byte);
  }
  return false;
}
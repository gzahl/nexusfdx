#include "NmeaSentenceParser.h"

#include <math.h>

#include "sensesp.h"

NmeaSentenceParser::NmeaSentenceParser(Stream* rx_stream)
    : Sensor() {
  rx_stream_ = rx_stream;
  current_state = &NmeaSentenceParser::state_start;
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

void NmeaSentenceParser::handle(char c) { (this->*(current_state))(c); }

void NmeaSentenceParser::state_start(char c) {
  switch (c) {
    case '$':
      cur_offset = 0;
      cur_term = 0;
      current_state = &NmeaSentenceParser::state_in_term;
      parity = 0;
      break;
    default:
      // anything else can be ignored
      break;
  }
}

void NmeaSentenceParser::state_in_term(char c) {
  switch (c) {
    case ',':
    case '*':
      if (cur_offset < INPUT_BUFFER_LENGTH) {
        // split terms with 0 to help further processing
        buffer[cur_offset++] = 0;
      } else {
        current_state = &NmeaSentenceParser::state_start;
        break;
      }
      if (cur_term < MAX_TERMS) {
        // advance term offset
        term_offsets[++cur_term] = cur_offset;
      } else {
        current_state = &NmeaSentenceParser::state_start;
        break;
      }
      if (c == '*') {
        current_state = &NmeaSentenceParser::state_in_checksum;
      } else {
        parity ^= c;
      }
      break;
    case '\r':
    case '\n':
      // end of sentence before checksum has been read
      buffer[cur_offset++] = 0;
      current_state = &NmeaSentenceParser::state_start;
      break;
    case '*':
      current_state = &NmeaSentenceParser::state_in_checksum;
    default:
      // read term characters
      buffer[cur_offset++] = c;
      parity ^= c;
      break;
  }
}

void NmeaSentenceParser::state_in_checksum(char c) {
  switch (c) {
    case ',':
    case '*':
      // there shouldn't be new terms after the checksum
      current_state = &NmeaSentenceParser::state_start;
    case '\r':
    case '\n':
      // end of sentence
      buffer[cur_offset++] = 0;
      if (!validate_checksum()) {
        current_state = &NmeaSentenceParser::state_start;
        return;
      }
      // call the relevant sentence parser
      if (sentence_parsers.find(buffer) == sentence_parsers.end()) {
        debugD("Parser not found for sentence %s", buffer);
      } else {
        sentence_parsers[buffer]->parse(buffer, term_offsets, cur_term + 1,
                                        sentence_parsers);
      }
      current_state = &NmeaSentenceParser::state_start;
      break;
    default:
      // read term characters
      buffer[cur_offset++] = c;
      break;
  }
}

bool NmeaSentenceParser::validate_checksum() {
  char* checksum_str = buffer + term_offsets[cur_term];
  int checksum;
  sscanf(checksum_str, "%2x", &checksum);
  return this->parity == checksum;
}

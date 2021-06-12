
#include "transforms/NmeaMessage.h"

NmeaMessage::NmeaMessage(char relativeOrTrue, String config_path)
    : Transform<Wind, String>(config_path), relativeOrTrue_(relativeOrTrue) {}

void NmeaMessage::set_input(Wind wind, uint8_t inputChannel) {
  this->emit(writeSentenceMWV(relativeOrTrue_, wind.direction, wind.speed));
}

String NmeaMessage::writeSentenceMWV(char trueOrApparent, float direction, float windspeed) {
  char buf[81];
  sprintf(buf, "$MWV,%f,%c,%f,N,A", direction, trueOrApparent, windspeed);
  return calcChecksum(buf);
}

String NmeaMessage::calcChecksum(char *nmea_data) {
  int crc = 0;
  int i;

  // the first $ sign
  for (i = 1; i < strlen(nmea_data); i++) {
    crc ^= nmea_data[i];
  }

  char chksumstr[3];
  sprintf(chksumstr, "%02x", crc);
  String sentence(nmea_data);
  sentence.concat("*");
  sentence.concat(chksumstr[0]);
  sentence.concat(chksumstr[1]);
  sentence.concat("\r\n");
  return sentence;
}
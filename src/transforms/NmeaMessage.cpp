
#include "transforms/NmeaMessage.h"

NmeaMessage::NmeaMessage(MessageType messageType, String config_path)
    : Transform<float, String>(config_path), messageType(messageType) {}

void NmeaMessage::set_input(float input, uint8_t inputChannel) {
  inputs[inputChannel] = input;
  received |= 1 << inputChannel;
  switch (messageType) {
    case (TRUE_WIND):
      if (received == 0b11) {
        received = 0;
        this->emit(writeSentenceMWV('T', inputs[0], inputs[1]));
      }
      break;
    case (APPARENT_WIND):
      if (received == 0b11) {
        received = 0;
        this->emit(writeSentenceMWV('R', inputs[0], inputs[1]));
      }
      break;
    case (WATER_TEMPERATURE):
      if (received == 0b1) {
        received = 0;
        this->emit(writeSentenceMTW(inputs[0]));
      }
      break;
  }
}

String NmeaMessage::writeSentenceMWV(char trueOrApparent, float direction,
                                     float windspeed) {
  char buf[81];
  sprintf(buf, "$MWV,%.1f,%c,%.1f,N,A", direction, trueOrApparent, windspeed);
  return calcChecksum(buf);
}

String NmeaMessage::writeSentenceMTW(float waterTemperature) {
  char buf[81];
  sprintf(buf, "$MTW,%.1f,C", waterTemperature);
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
  sprintf(chksumstr, "%02X", crc);
  String sentence(nmea_data);
  sentence.concat("*");
  sentence.concat(chksumstr[0]);
  sentence.concat(chksumstr[1]);
  sentence.concat("\r\n");
  return sentence;
}

#include "transforms/NmeaMessage.h"

template <typename T>
NmeaMessage<T>::NmeaMessage(MessageType messageType, String config_path)
    : sensesp::Transform<T, String>(config_path), messageType(messageType) {
  strcpy(deviceId, "--");
}

template <typename T>
void NmeaMessage<T>::set_input(T input, uint8_t inputChannel) {
  inputs[inputChannel] = input;
  received |= 1 << inputChannel;
  switch (messageType) {
    case (NMEA_MWV_TRUE):
      if (received == 0b11) {
        received = 0;
        this->emit(writeSentenceMWV('T', inputs[0], inputs[1]));
      }
      break;
    case (NMEA_MWV_RELATIVE):
      if (received == 0b11) {
        received = 0;
        this->emit(writeSentenceMWV('R', inputs[0], inputs[1]));
      }
      break;
    case (NMEA_MTW):
      if (received == 0b1) {
        received = 0;
        this->emit(writeSentenceMTW(inputs[0]));
      }
      break;
    case (NMEA_XDR_VOLTAGE):
      if (received == 0b1) {
        received = 0;
        this->emit(writeSentenceXDR('V', 'V', "BUS_VOLTAGE", inputs[0]));
      }
      break;
    case (NMEA_XDR_SIGNALSTRENGTH):
      if (received == 0b1) {
        received = 0;
        this->emit(writeSentenceXDR('S', 'P', "SIGNAL_STRENGTH", inputs[0]));
      }
      break;
    case (NMEA_XDR_DMP_ACCURACY):
      if(received == 0b1) {
        received = 0;
        this->emit(writeSentenceXDR('S', 'P', "DMP_ACCURACY", inputs[0]));
      }
      break;
    case (NMEA_XDR_PITCH):
      if (received == 0b1) {
        received = 0;
        this->emit(writeSentenceXDR('A', 'D', "PITCH", inputs[0]));
      }
      break;
    case (NMEA_XDR_ROLL):
      if (received == 0b1) {
        received = 0;
        this->emit(writeSentenceXDR('A', 'D', "ROLL", inputs[0]));
      }
      break;
    case (NMEA_DPT):
      if (received == 0b1) {
        received = 0;
        this->emit(writeSentenceDPT(inputs[0], 0.));
      }
      break;
    case (NMEA_HDM):
      if (received == 0b1) {
        received = 0;
        this->emit(writeSentenceHDM(inputs[0]));
      }
    break;
    case (NMEA_ROT):
      if (received == 0b1) {
        received = 0;
        this->emit(writeSentenceROT(inputs[0]));
      }
    break;
  }
}

template <typename T>
String NmeaMessage<T>::writeSentenceMWV(char trueOrApparent, T direction,
                                        T windspeed) {
  char buf[81];
  sprintf(buf, "$%2sMWV,%.1f,%c,%.1f,N,A", deviceId, direction, trueOrApparent,
          windspeed);
  return calcChecksum(buf);
}

template <typename T>
String NmeaMessage<T>::writeSentenceMTW(T waterTemperature) {
  char buf[81];
  sprintf(buf, "$%2sMTW,%.1f,C", deviceId, waterTemperature);
  return calcChecksum(buf);
}

template <typename T>
String NmeaMessage<T>::writeSentenceXDR(char type, char unit, const char* name,
                                        T value) {
  char buf[81];
  sprintf(buf, "$%2sXDR,%c,%.2f,%c,%s", deviceId, type, value, unit, name);
  return calcChecksum(buf);
}

template <typename T>
String NmeaMessage<T>::writeSentenceDPT(T depth, T offset) {
  char buf[81];
  sprintf(buf, "$%2sDPT,%.1f,%.1f", deviceId, depth, offset);
  return calcChecksum(buf);
}

template <typename T>
String NmeaMessage<T>::writeSentenceHDM(T heading) {
  char buf[81];
  sprintf(buf, "$%2sHDM,%.1f,M", deviceId, heading);
  return calcChecksum(buf);
}

template <typename T>
String NmeaMessage<T>::writeSentenceROT(T rateOfTurn) {
  char buf[81];
  sprintf(buf, "$%2sROT,%.1f,A", deviceId, rateOfTurn);
  return calcChecksum(buf);
}


template <typename T>
String NmeaMessage<T>::calcChecksum(char* nmea_data) {
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
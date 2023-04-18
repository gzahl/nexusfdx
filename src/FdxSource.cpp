#include "FdxSource.h"

#include "sensesp.h"

#include "ReactESP.h"
extern reactesp::ReactESP app;

using sensesp::Debug;

FdxSource::FdxSource(SoftwareSerial *rx_stream) : Sensor(), fdxParser() {
  rx_stream_ = rx_stream;
  len = 0;
}

void FdxSource::start() {
  // enable reading the serial port
  debugI("Enabling FdxSource!");
  app.onAvailable(*rx_stream_, [this]() {
    while (rx_stream_->available()) {
      byte = rx_stream_->read();
      // Serial.printf("0x%x", byte);s
      if (rx_stream_->readParity() && len > 0) {
        readMessage(message, len);
        len = 0;
      }
      message[len++] = reverse(byte);
    }
  });
}

void FdxSource::readMessage(unsigned char *msg, unsigned char len) {
  // printMessage(message, len);
  bool isSender = (msg[0] >> 7) == 1;
  if (!isSender) {
    emitRawMessage(msg, len);
  }
  fdxParser.parse(msg, len);
  switch (fdxParser.data.type) {
    case (APPARENT_WIND):
      data.apparantWind.angle.emit(fdxParser.data.apparantWind.angle);
      data.apparantWind.speed.emit(fdxParser.data.apparantWind.speed);
      break;
    case (TRUE_WIND):
      data.trueWind.angle.emit(fdxParser.data.trueWind.angle);
      data.trueWind.speed.emit(fdxParser.data.trueWind.speed);
      break;
    case (TEMPERATURE):
      data.temperature.emit(fdxParser.data.temperature);
      break;
    case (VOLTAGE):
      data.voltage.emit(fdxParser.data.voltage);
      break;
    case (WIND_TRANSDUCER_SIGNAL):
      data.signalStrength.emit(fdxParser.data.signalStrength);
      break;
    case (DEPTH):
      data.depth.emit(fdxParser.data.depth);
      break;
    case (UNKNOWN):
      break;
    default:
      break;
  }
}

unsigned char FdxSource::reverse(unsigned char b) {
  b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
  b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
  b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
  return b;
}

void FdxSource::printMessage(unsigned char *msg, unsigned char msglen) {
  #ifndef DEBUG_DISABLED
  for (unsigned char i = 0; i < msglen; i++) {
    rdebugV("0x%x ", msg[i]);
  }
  debugV("");
  #endif
}

void FdxSource::emitRawMessage(unsigned char *msg, unsigned char msglen) {
  String line;
  char buf[11];
  sprintf(buf, "%010lu", millis());
  line.concat(buf);
  for (unsigned char i = 0; i < msglen; i++) {
    sprintf(buf, " %02X", msg[i]);
    line.concat(buf);
  }
  line.concat("\r\n");
  data.rawMessage.emit(line);
}

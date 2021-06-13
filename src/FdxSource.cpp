#include "FdxSource.h"

#include "sensesp.h"

FdxSource::FdxSource(SoftwareSerial *rx_stream) : Sensor(), fdxParser(&fdxData) {
  rx_stream_ = rx_stream;
  len = 0;
}

void FdxSource::enable() {
  // enable reading the serial port
  Serial.println("Enabling FdxSource!");
  app.onAvailable(*rx_stream_, [this]() {
    while (rx_stream_->available()) {
      byte = rx_stream_->read();
      // Serial.printf("0x%x", byte);s
      if (rx_stream_->readParity() && len > 0) {
        // printMessage(message, len);
        bool isSender = (msg[0] >> 7) == 1;
        if(!isSender) {
          emitRawMessage(message, len);
        }
        fdxParser.parse(message,len);
        len = 0;
      }
      message[len++] = reverse(byte);
    }
  });
}

unsigned char FdxSource::reverse(unsigned char b) {
  b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
  b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
  b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
  return b;
}

void FdxSource::printMessage(unsigned char *msg, unsigned char msglen) {
  for (unsigned char i = 0; i < msglen; i++) {
    Serial.printf("0x%x ", msg[i]);
  }
  Serial.printf("\n");
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
  fdxData.rawMessage.emit(line);
}


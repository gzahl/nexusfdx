#include "FdxParser.h"

FdxParser::FdxParser() {}

void FdxParser::parse(unsigned char *msg, unsigned char len) {
  data.type = UNKNOWN;
  bool isSender = (msg[0] >> 7) == 1;
  unsigned char headerPayload = msg[0] & 0b01111111;
  if (isSender) {
    if (len != 1) return;
    if (headerPayload != 2 && headerPayload != 127 && headerPayload != 16)
      printf("New Sender with address: %d\n", headerPayload);
  } else {
    // At least one header byte and one checksum byte
    if (len < 2) {
      // printf("Data message too short (len=%u): ",len);
      // readData(headerPayload, msg, len);
      return;
    }

    unsigned char payloadLen = len - 2;
    unsigned char *payload = msg + 1;

    bool correctChksum = message[len - 1] == calcChksum(msg, len);
    if (!correctChksum) {
      // printf("Wrong chksum: ");
      // readData(headerPayload, payload, payloadLen);
      return;
    }
    switch (headerPayload) {
      case (0):
        if (payloadLen != 2) return;
        break;
      case (1):
        // With resting wind transducer
        // the last two bytes keep the last values,
        // first two ones zero, e.g.
        // 0x0 0x0 0xde 0x98
        // Seems similar two Msg18, but keeps last read windspeed and direction
        if (payloadLen != 4) return;
        // readData(headerPayload, payload, payloadLen);
        // readMsg18(payload);
        break;
      case (3):
        assert(payloadLen == 1);
      case (4):
        if (payloadLen != 3) return;
        break;
      case (7):
        if (payloadLen != 3) return;
        break;
      case (8):
        assert(payloadLen == 1);
        break;
      case (9):
        assert(payloadLen == 1);
        break;
      case (17):
        // Always 0x00s 0x00?
        if (payloadLen != 2) return;
        break;
      case (18):
        // Wind direction & speed + unknown byte
        if (payloadLen != 4) return;
        // readData(headerPayload, payload, payloadLen);
        readMsg18(payload);
        break;
      case (21):
        // Only the first bytes seem to vary.
        // 0x34 0xef 0xff 0xff
        // Second byte can be 0xee, 0xef, 0xed, but mostly 0exef
        // Last two bytes always 0xff 0xff?
        // Updates more often if wind sensor is active, but value seems to jump
        // around
        assert(payloadLen == 4);
        // readData(headerPayload, payload, payloadLen);
        readMsg21(payload);
        break;
      case (26):
        // The payload is constant 0xb 0x24 0xff 0x00
        if (payloadLen != 4) return;
        break;
      case (28):
        // Only seen every 20-30s or so
        // constant 0x1c 0x21 0x1b?
        assert(payloadLen == 3);
        break;
      case (112):
        // Wind Transducer Signal Strength + two unknown bytes
        // More repititions if there is wind
        // example: 0x89 0xcc 0x80
        assert(payloadLen == 3);
        // readData(headerPayload, payload, payloadLen);
        readMsg112(payload);
        break;
      default:
        //Serial.printf("Unknown message with key %u of len=%u\n", headerPayload,
        //              payloadLen);
        break;
    }
  }
}

void FdxParser::readData(unsigned char messageId, unsigned char *payload,
                         unsigned char len) {
  //Serial.printf("[%d] ", messageId);
  //printMessage(payload, len);
}

void FdxParser::readMsg18(uint8_t *payload) {
  if (payload[0] == 0x0 && payload[1] == 0x0 && payload[2] == 0x0 &&
      payload[3] == 0x20) {
    // No wind, standing still
    //Serial.printf("No wind\n");
  }
  uint16_t windspeedBytes =
      (uint16_t)(payload[1]) << 8 | (uint16_t)(payload[2]);
  // uint16_t direction = (uint16_t)(payload[2]) << 8 | (uint16_t)(payload[3]);
  uint8_t angleByte = payload[3];
  float angle = (float)angleByte * 360. / 255.;
  // vavg.Insert((float)windspeedBytes);
  // float windspeed = vavg.GetAverage() * 1.e-2 * 1.94;
  float speed = (float)windspeedBytes * 1.e-2 * 1.94;

  //Serial.printf("Direction[°]: %f, Speed[m/s?]: %f, Unknown: %u\n", angle,
  //              speed, payload[0]);
  // fdxData->relativeWind.direction.emit(direction);
  // fdxData->relativeWind.speed.emit(speed);
  data.type = WIND;
  data.relativeWind.angle = angle;
  data.relativeWind.speed = speed;
}

void FdxParser::readMsg112(uint8_t *payload) {
  uint8_t signalStrengthByte = payload[1];
  float signalStrength = (float)signalStrengthByte / (float)0xFF * 100.;
  //Serial.printf("Wind Transducer signal strength[%%]: %f\n", signalStrength);

  // payload[2] might be a flag of some sorts?
  // seen: Mostly 0x80, somtimes: 0xad, 0xf1, 0xcb 0xc6
}

void FdxParser::readMsg21(uint8_t *payload) {
  uint8_t unknownByte = payload[0];
  //Serial.printf("Unknown: %u\n", unknownByte);
}

unsigned char FdxParser::calcChksum(unsigned char *msg, unsigned char len) {
  unsigned char chksum = msg[0];
  for (unsigned char i = 1; i < len - 1; i++) {
    chksum = chksum ^ msg[i];
  }
  return chksum;
}

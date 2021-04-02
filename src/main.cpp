#include <Arduino.h>
#include <SoftwareSerial.h>

SoftwareSerial swSerial;

unsigned char reverse(unsigned char);
void printMessage(unsigned char *msg, unsigned char len);
void readMessage(unsigned char *msg, unsigned char len);
void readData(unsigned char messageId, unsigned char *msg, unsigned char len);
void readMsg18(uint8_t *payload);
void readMsg21(uint8_t *payload);
void readMsg112(uint8_t *payload);
unsigned char calcChksum(unsigned char *msg, unsigned char len);
unsigned char message[50];
unsigned char len = 0;

void setup()
{
  Serial.begin(115200);
  //swSerial.begin(9600, SWSERIAL_8S1, GPIO_NUM_21);
  //pinMode(GPIO_NUM_26, INPUT);
  //pinMode(GPIO_NUM_18, OUTPUT);

  swSerial.begin(9600, SWSERIAL_8S1, GPIO_NUM_26, GPIO_NUM_18);
  // put your setup code here, to run once:
  Serial.printf("Hello, starting now..\n");
}

void loop()
{
  unsigned char byte;
  while (swSerial.available())
  {
    byte = swSerial.read();
    //Serial.printf("0x%x", byte);
    if (swSerial.readParity() && len > 0)
    {
      //printMessage(message, len);
      readMessage(message, len);
      len = 0;
    }
    message[len++] = reverse(byte);
  }
}

unsigned char reverse(unsigned char b)
{
  b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
  b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
  b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
  return b;
}

void printMessage(unsigned char *msg, unsigned char msglen)
{
  for (unsigned char i = 0; i < msglen; i++)
  {
    Serial.printf("0x%x ", msg[i]);
  }
  Serial.printf("\n");
}

unsigned char calcChksum(unsigned char *msg, unsigned char len)
{
  unsigned char chksum = message[0];
  for (unsigned char i = 1; i < len - 1; i++)
  {
    chksum = chksum ^ message[i];
  }
  return chksum;
}

void readMessage(unsigned char *msg, unsigned char len)
{
  assert(len > 0);
  bool isSender = (msg[0] >> 7) == 1;
  unsigned char headerPayload = msg[0] & 0b01111111;
  if (isSender)
  {
    if (len != 1)
      return;
    if (headerPayload != 2 && headerPayload != 127)
      printf("New Sender with address: %d\n", headerPayload);
  }
  else
  {
    // At least one header byte and one checksum byte
    if(len<2) {
      //printf("Data message too short (len=%u): ",len);
      //readData(headerPayload, msg, len);
      return;
    }
      
    unsigned char payloadLen = len - 2;
    unsigned char *payload = msg + 1;

    bool correctChksum = message[len - 1] == calcChksum(msg, len);
    if (!correctChksum) {
      //printf("Wrong chksum: ");
      //readData(headerPayload, payload, payloadLen);
      return;
    }
    switch (headerPayload)
    {
    case (0):
      if (payloadLen != 2)
        return;
      break;
    case (1):
      // With resting wind transducer
      // the last two bytes keep the last values,
      // first two ones zero, e.g.
      // 0x0 0x0 0xde 0x98
      // Seems similar two Msg18, but keeps last read windspeed and direction
      if (payloadLen != 4)
        return;
      //readData(headerPayload, payload, payloadLen);
      readMsg18(payload);
      break;
    case (3):
      assert(payloadLen == 1);
    case (4):
      if (payloadLen != 3)
        return;
      break;
    case (7):
      if (payloadLen != 3)
        return;
      break;
    case (8):
      assert(payloadLen == 1);
      break;
    case (9):
      assert(payloadLen == 1);
      break;
    case (17):
      // Always 0x00s 0x00?
      if (payloadLen != 2)
        return;
      break;
    case (18):
      // Wind direction & speed + unknown byte
      if (payloadLen != 4)
        return;
      //readData(headerPayload, payload, payloadLen);
      readMsg18(payload);
      break;
    case (21):
      // Only the first bytes seem to vary.
      // 0x34 0xef 0xff 0xff
      // Second byte can be 0xee, 0xef, 0xed, but mostly 0exef
      // Last two bytes always 0xff 0xff?
      // Updates more often if wind sensor is active, but value seems to jump around
      assert(payloadLen == 4);
      //readData(headerPayload, payload, payloadLen);
      readMsg21(payload);
      break;
    case (26):
      // The payload is constant 0xb 0x24 0xff 0x00
      if (payloadLen != 4)
        return;
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
      //readData(headerPayload, payload, payloadLen);
      readMsg112(payload);
      break;
    default:
      Serial.printf("Unknown message with key %u of len=%u\n", headerPayload, payloadLen);
    }
  }
}

void readData(unsigned char messageId, unsigned char *payload, unsigned char len)
{
  Serial.printf("[%d] ", messageId);
  printMessage(payload, len);
}

void readMsg18(uint8_t *payload)
{
  if(payload[0]==0x0 && payload[1]==0x0 && payload[2]==0x0 && payload[3] == 0x20) {
    // No wind, standing still
    Serial.printf("No wind\n");
  }
  uint16_t windspeedBytes = (uint16_t)(payload[1]) << 8 | (uint16_t)(payload[2]);
  //uint16_t direction = (uint16_t)(payload[2]) << 8 | (uint16_t)(payload[3]);
  uint8_t directionByte = payload[3];
  float direction = (float)directionByte * 360./255.;
  float windspeed = (float)windspeedBytes * 1e-2;

  Serial.printf("Direction[°]: %f, Speed[m/s?]: %f, Unknown: %u\n", direction, windspeed, payload[0]);
}

void readMsg112(uint8_t* payload)
{
  uint8_t signalStrengthByte = payload[1];
  float signalStrength = (float)signalStrengthByte / (float)0xFF * 100.;
  Serial.printf("Wind Transducer signal strength[%%]: %f\n", signalStrength);

  // payload[2] might be a flag of some sorts?
  // seen: Mostly 0x80, somtimes: 0xad, 0xf1, 0xcb 0xc6
}

void readMsg21(uint8_t *payload) {
  uint8_t unknownByte = payload[0];
  Serial.printf("Unknown: %u\n", unknownByte);
}
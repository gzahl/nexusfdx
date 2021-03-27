#include <Arduino.h>
#include <SoftwareSerial.h>

SoftwareSerial swSerial;

unsigned char reverse(unsigned char);
void printMessage(unsigned char *msg, unsigned char len);
void readMessage(unsigned char *msg, unsigned char len);
void readData(unsigned char messageId, unsigned char *msg, unsigned char len);
void readMsg18(unsigned char *payload);
void readMsg112(uint8_t *payload);
unsigned char calcChksum(unsigned char *msg, unsigned char len);
unsigned char message[50];
unsigned char len = 0;

void setup()
{
  Serial.begin(115200);
  swSerial.begin(9600, SWSERIAL_8S1, GPIO_NUM_21);
  // put your setup code here, to run once:
}

void loop()
{
  unsigned char byte;
  while (swSerial.available())
  {
    byte = swSerial.read();
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
    bool correctChksum = message[len - 1] == calcChksum(msg, len);
    if (!correctChksum)
      return;
    //assert(correctChksum);
    unsigned char payloadLen = len - 2;
    unsigned char *payload = msg + 1;
    switch (headerPayload)
    {
    case (0):
      if (payloadLen != 2)
        return;
      break;
    case (1):
      if (payloadLen != 4)
        return;
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
      // Always 0x11 0x00?
      if (payloadLen != 2)
        return;
      break;
    case (18):
      // Is correlated to wind speed, e.g.
      // No wind: 0x12 0x0 0x0 0x0
      // First byte always 0x12?
      // last two bytes seem to belong together (two byte number)
      if (payloadLen != 4)
        return;
      readData(headerPayload, payload, payloadLen);
      readMsg18(payload);
      break;
    case (21):
      // Only the middle two bytes seem to vary.
      // Updates more often if wind sensor is active
      assert(payloadLen == 4);
      break;
    case (26):
      // The payload is constant 0x1a 0xb 0x24 0xff
      assert(payloadLen == 4);
      break;
    case (28):
      // Only seen every 20-30s or so
      // constant 0x1c 0x21 0x1b?
      assert(payloadLen == 3);
      break;
    case (112):
      // More repititions if there is wind
      // but not related to speed or direction
      // example: 0x70 0x89 0xcc
      // First two bytes seem constant.
      // Maybe signal strength?
      assert(payloadLen == 3);
      //readData(headerPayload, payload, payloadLen);
      //readMsg112(payload);
      break;
    default:
      Serial.printf("Unkown message with key %d of len=%d\n", headerPayload, payloadLen);
    }
  }
}

void readData(unsigned char messageId, unsigned char *payload, unsigned char len)
{
  Serial.printf("[%d] ", messageId);
  printMessage(payload, len);
}

void readMsg18(unsigned char *payload)
{
  uint16_t direction = (uint16_t)(payload[0]) << 8 | (uint16_t)(payload[1]);
  uint16_t windspeed = (uint16_t)(payload[2]) << 8 | (uint16_t)(payload[3]);
  Serial.printf("Windspeed: %hhu, %hhu\n", direction, windspeed);
}

void readMsg112(uint8_t* payload)
{
  uint8_t signalStrength = payload[2];
  Serial.printf("%03hhu\n", signalStrength);
}
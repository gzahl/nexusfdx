#include <Arduino.h>
#include <SoftwareSerial.h>
#include <fdx.h>

SoftwareSerial swSerial;
SoftwareSerial swSerialGps;
SoftwareSerial swSerialNmea0;
SoftwareSerial swSerialNmea1;


//unsigned char gpsmessage[100];
unsigned char gpsmessagelen = 0;
std::vector<uint8_t> gpsmsg(100);
std::vector<uint8_t> aismsg(100);
unsigned char len = 0;

static const bool ENABLE_NMEA2 = false;
static const bool ENABLE_GPS = true;
static const bool ENABLE_NMEA0 = true; // AIS Input
static const bool ENABLE_NMEA1 = true; // DSC Input, GPS Output


void setup()
{
  Serial.begin(115200);
  Serial.printf("\nHello, starting now..\n");

  // Use HardwareSerial2 for GPS, since Baudrate of 115200 is problamatic with
  // SoftwareSerial and setting the baudrate is not working
  Serial2.begin(115200, SERIAL_8N1, GPIO_NUM_23, GPIO_NUM_19);

  aismsg.clear();
  gpsmsg.clear();
  

  //pinMode(GPIO_NUM_19, PULLUP);
  delay(1000);
  //Serial2.print("$PUBX,41,1,3,2,115200,0*1D\r\n");
  //Serial2.print("$PUBX,41,1,3,2,38400,0*25\r\n");
  //Serial2.print("$PUBX,41,1,3,2,9600,0*15\r\n");
  Serial2.print("$PUBX,41,1,3,2,4800,0*16\r\n");
  Serial2.flush();
  delay(100);
  Serial2.clearWriteError();
  Serial2.end();
  delay(100);
  //Serial2.begin(4800, SERIAL_8N1, GPIO_NUM_23, GPIO_NUM_19);

  

  //swSerial.begin(9600, SWSERIAL_8S1, GPIO_NUM_21);
  //pinMode(GPIO_NUM_26, INPUT);
  //pinMode(GPIO_NUM_18, OUTPUT);

  swSerial.begin(9600, SWSERIAL_8S1, GPIO_NUM_26, GPIO_NUM_18);
  swSerialGps.begin(4800, SWSERIAL_8N1, GPIO_NUM_23, GPIO_NUM_19);
  swSerialNmea0.begin(38400, SWSERIAL_8N1, GPIO_NUM_33, GPIO_NUM_14);
  swSerialNmea1.begin(38400, SWSERIAL_8N1, GPIO_NUM_5, GPIO_NUM_13);

}

bool readNmea(std::vector<uint8_t> &buffer, uint8_t byte) {
    if(byte == '$') {
      buffer.clear();
    }
    buffer.push_back(byte);
    if(byte == '\n') {
      return true;
    }
    return false;
}


void loop()
{
  unsigned char byte;
  while (ENABLE_NMEA2 && swSerial.available())
  {
    byte = swSerial.read();
    //Serial.printf("0x%x", byte);s
    if (swSerial.readParity() && len > 0)
    {
      //printMessage(message, len);
      readMessage(message, len);
      len = 0;
    }
    message[len++] = reverse(byte);
  }

  while(ENABLE_GPS && swSerialGps.available()) {
    if(readNmea(gpsmsg, swSerialGps.read())) {
      for(auto c : gpsmsg){
        Serial.write(c);
        swSerialNmea1.write(c);
      }
    }
  }

  while(ENABLE_NMEA0 && swSerialNmea0.available()) {
    if(readNmea(aismsg, swSerialNmea0.read())) {
      for(auto c : aismsg) {
        Serial.write(c);
      }
    }
  }

    while(ENABLE_NMEA1 && swSerialNmea1.available()) {
  //  while(swSerialNmea0.available()){
    byte = swSerialNmea1.read();
    Serial.write(byte);
  }
}
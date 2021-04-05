#include <Arduino.h>
#include <SoftwareSerial.h>
#include <fdx.h>
#include <WiFi.h>
#include <AsyncUDP.h>

SoftwareSerial swSerial;
SoftwareSerial swSerialGps;
SoftwareSerial swSerialNmea0;
SoftwareSerial swSerialNmea1;

void configureUbloxM8Gps();

std::vector<uint8_t> gpsmsg(100);
std::vector<uint8_t> aismsg(100);
std::vector<uint8_t> dscmsg(100);
unsigned char len = 0;

static const bool ENABLE_NMEA2 = false;
static const bool ENABLE_GPS = true;
static const bool ENABLE_NMEA0 = true; // AIS Input
static const bool ENABLE_NMEA1 = true; // DSC Input, GPS Output


const char* ssid = "Schmuddelwetter_24G";
const char* password = "9568164986244857";

AsyncUDP udp;

void setup()
{
  Serial.begin(115200);
  Serial.printf("\nHello, starting now..\n");
  
  configureUbloxM8Gps();

  aismsg.clear();
  gpsmsg.clear();
  dscmsg.clear();
   
  Serial.printf("Connecting to Wifi with ssid '%s'.\n", ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
      if (WiFi.waitForConnectResult() != WL_CONNECTED) {
        Serial.println("WiFi Failed");
        while(1) {
            delay(1000);
        }
    } else {
      Serial.println("WiFi connected.");
    }



  //swSerial.begin(9600, SWSERIAL_8S1, GPIO_NUM_21);
  //pinMode(GPIO_NUM_26, INPUT);
  //pinMode(GPIO_NUM_18, OUTPUT);

  swSerial.begin(9600, SWSERIAL_8S1, GPIO_NUM_26, GPIO_NUM_18);
  swSerialGps.begin(9600, SWSERIAL_8N1, GPIO_NUM_23, GPIO_NUM_19);
  swSerialNmea0.begin(38400, SWSERIAL_8N1, GPIO_NUM_33, GPIO_NUM_14);
  swSerialNmea1.begin(38400, SWSERIAL_8N1, GPIO_NUM_5, GPIO_NUM_13);

}

void configureUbloxM8Gps() {
  // Use HardwareSerial2 to configure GPS, since Baudrate of 115200 is problamatic with
  // SoftwareSerial and setting the baudrate is not working
  Serial2.begin(115200, SERIAL_8N1, GPIO_NUM_23, GPIO_NUM_19);
  //pinMode(GPIO_NUM_19, PULLUP);
  delay(1000);
  //Serial2.print("$PUBX,41,1,3,2,115200,0*1D\r\n");
  //Serial2.print("$PUBX,41,1,3,2,38400,0*25\r\n");
  Serial2.print("$PUBX,41,1,3,2,9600,0*15\r\n");
  //Serial2.print("$PUBX,41,1,3,2,4800,0*16\r\n");
  Serial2.flush();
  delay(100);
  Serial2.clearWriteError();
  Serial2.end();
  delay(100);
}


bool readNmea(std::vector<uint8_t> &buffer, uint8_t byte) {
    if(byte == '$') {
      buffer.clear();
    }
    buffer.push_back(byte);
    if(byte == '\n') {
      buffer.push_back('\0');
      return true;
    }
    return false;
}

char* getLine(std::vector<uint8_t> buffer) {
  return reinterpret_cast<char*>(buffer.data());
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
      Serial.print(getLine(gpsmsg));
      swSerialNmea1.print(getLine(gpsmsg));
      udp.broadcastTo(getLine(gpsmsg), 2000);
    }
  }

  while(ENABLE_NMEA0 && swSerialNmea0.available()) {
    if(readNmea(aismsg, swSerialNmea0.read())) {
      Serial.print(getLine(aismsg));
      udp.broadcastTo(getLine(aismsg), 2000);
    }
  }

    while(ENABLE_NMEA1 && swSerialNmea1.available()) {
      if(readNmea(dscmsg, swSerialNmea1.read())) {
        Serial.print(getLine(dscmsg));
        udp.broadcastTo(getLine(dscmsg), 2000);
    }
  }
}
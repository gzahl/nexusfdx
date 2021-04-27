#include <Arduino.h>
#include <AsyncUDP.h>
#include <SoftwareSerial.h>
#include <WiFi.h>

#include "SerialFdxListenerTask.h"
#include "SerialNmeaListenerTask.h"

void configureUbloxM8Gps();
static char *getLine(std::vector<uint8_t> buffer);

static const bool ENABLE_NMEA2 = true;
static const bool ENABLE_GPS = false;
static const bool ENABLE_NMEA0 = false; // AIS Input
static const bool ENABLE_NMEA1 = false; // DSC Input, GPS Output
static const bool ENABLE_WIFI = false;

const char *ssid = "Schmuddelwetter_24G";
const char *password = "9568164986244857";

AsyncUDP udp;

void setup() {
  Serial.begin(115200);
  Serial.printf("\nHello, starting now..\n");

  if (ENABLE_WIFI) {
    Serial.printf("Connecting to Wifi with ssid '%s'.\n", ssid);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    if (WiFi.waitForConnectResult() != WL_CONNECTED) {
      Serial.println("WiFi Failed");
      while (1) {
        delay(1000);
      }
    } else {
      Serial.println("WiFi connected.");
    }
  }

  // swSerial.begin(9600, SWSERIAL_8S1, GPIO_NUM_21);
  // pinMode(GPIO_NUM_26, INPUT);
  // pinMode(GPIO_NUM_18, OUTPUT);

  // AIS input
  if (ENABLE_NMEA0) {
    new SerialNmeaListenerTask(38400, SWSERIAL_8N1, GPIO_NUM_33, GPIO_NUM_14,
                               [](std::vector<uint8_t> &msg) {
                                 Serial.print(getLine(msg));
                                 udp.broadcastTo(getLine(msg), 2000);
                               });
  }

  // DSC Input, GPS Output
  if (ENABLE_NMEA1) {
    new SerialNmeaListenerTask(38400, SWSERIAL_8N1, GPIO_NUM_5, GPIO_NUM_13,
                               [](std::vector<uint8_t> &msg) {
                                 Serial.print(getLine(msg));
                                 udp.broadcastTo(getLine(msg), 2000);
                               });
  }

  // Nexus FDX Input
  if (ENABLE_NMEA2) {
    new SerialFdxListenerTask(9600, SWSERIAL_8S1, GPIO_NUM_26, GPIO_NUM_18,
                              [](std::vector<uint8_t> &msg) {});
  }

  if (ENABLE_GPS) {
    configureUbloxM8Gps();
    new SerialNmeaListenerTask(9600, SWSERIAL_8N1, GPIO_NUM_23, GPIO_NUM_19,
                               [](std::vector<uint8_t> &msg) {
                                 Serial.print(getLine(msg));
                                 // swSerialNmea1.print(getLine(msg));
                                 if (ENABLE_WIFI) {
                                   udp.broadcastTo(getLine(msg), 2000);
                                 }
                               });
  }
}

void configureUbloxM8Gps() {
  // Use HardwareSerial2 to configure GPS, since Baudrate of 115200 is
  // problamatic with SoftwareSerial and setting the baudrate is not working
  Serial2.begin(115200, SERIAL_8N1, GPIO_NUM_23, GPIO_NUM_19);
  // pinMode(GPIO_NUM_19, PULLUP);
  delay(1000);
  // Serial2.print("$PUBX,41,1,3,2,115200,0*1D\r\n");
  // Serial2.print("$PUBX,41,1,3,2,38400,0*25\r\n");
  Serial2.print("$PUBX,41,1,3,2,9600,0*15\r\n");
  // Serial2.print("$PUBX,41,1,3,2,4800,0*16\r\n");
  Serial2.flush();
  delay(100);
  Serial2.clearWriteError();
  Serial2.end();
  delay(100);
}

static char *getLine(std::vector<uint8_t> buffer) {
  return reinterpret_cast<char *>(buffer.data());
}

void loop() {}
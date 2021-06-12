#include <Arduino.h>
#include <AsyncUDP.h>
#include <SoftwareSerial.h>
#include <WiFi.h>
#include <ESPmDNS.h>

#include "sensesp.h"
#include "system/lambda_consumer.h"
#include "wiring_helpers.h"

#include "FdxSource.h"
#include "transforms/NmeaMessage.h"
#include "transforms/moving_average.h"

#include "NmeaSentenceSource.h"

void configureUbloxM8Gps();

static const bool ENABLE_GPS = true;
static const bool ENABLE_NMEA0 = true;     // Radio: AIS Input
static const bool ENABLE_NMEA1 = true;     // Radio: DSC Input, GPS Output
static const bool ENABLE_NMEA2 = true;     // Nexus FDX
static const bool ENABLE_ELITE4HDI = false; // GPS Input, AIS Output
static const bool ENABLE_WIFI_STA = false;

const char *ssid = "Schmuddelwetter_24G";
const char *password = "9568164986244857";
const char *ssid_svala = "Svala";
const char *password_svala = "8641009916";
const uint16_t BROADCAST_PORT = 2000;

static const gpio_num_t NMEA0_RX = GPIO_NUM_33;
static const gpio_num_t NMEA0_TX = GPIO_NUM_14;
static const gpio_num_t NMEA1_RX = GPIO_NUM_5;
static const gpio_num_t NMEA1_TX = GPIO_NUM_13;
static const gpio_num_t NMEA2_RX = GPIO_NUM_26;
static const gpio_num_t NMEA2_TX = GPIO_NUM_18;
static const gpio_num_t NMEA3_RX = GPIO_NUM_25;
static const gpio_num_t NMEA3_TX = GPIO_NUM_27;
static const gpio_num_t NMEA4_RX = GPIO_NUM_32;
static const gpio_num_t NMEA4_TX = GPIO_NUM_12;
static const gpio_num_t NMEA5_RX = GPIO_NUM_17;
static const gpio_num_t NMEA5_TX = GPIO_NUM_4;
static const gpio_num_t NMEA6_RX = GPIO_NUM_0;
static const gpio_num_t NMEA6_TX = GPIO_NUM_16;
static const gpio_num_t GPS_RX = GPIO_NUM_19;
static const gpio_num_t GPS_TX = GPIO_NUM_23;

int bufCapacity = 80;
int isrBufCapacity = 20;

AsyncUDP udp;

SoftwareSerial *swSerial[8];

void setupApp() {
  Serial.begin(115200);
  Serial.printf("\nHello, starting now..\n");

  for (int i = 0; i < 8; i++) {
    swSerial[i] = NULL;
  }

  if (ENABLE_WIFI_STA) {
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
  } else {
    Serial.printf("Wifi AP with ssid '%s'.\n", ssid_svala);
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid_svala, password_svala);
  }

  if(!MDNS.begin("svala")) {
     Serial.println("Error starting mDNS");
     return;
}
  
  // swSerial.begin(9600, SWSERIAL_8S1, GPIO_NUM_21);
  // pinMode(GPIO_NUM_26, INPUT);
  // pinMode(GPIO_NUM_18, OUTPUT);

  if (ENABLE_ELITE4HDI) {
    // Elite4HDI_TX
    swSerial[4] = new SoftwareSerial();
    swSerial[4]->begin(38400, SWSERIAL_8N1, NMEA4_RX, NMEA4_TX, false, bufCapacity, isrBufCapacity);
  }

  // AIS input
  if (ENABLE_NMEA0) {
    swSerial[0] = new SoftwareSerial();
    swSerial[0]->begin(38400, SWSERIAL_8N1, NMEA0_RX, NMEA0_TX, false, bufCapacity, isrBufCapacity);
    auto *nmeaSentenceSource = new NmeaSentenceSource(swSerial[0]);
    auto nmeaSentenceReporter = new LambdaConsumer<String>([](String msg) {
      Serial.print(msg);
      if (swSerial[4])
        swSerial[4]->print(msg);
      udp.broadcastTo(msg.c_str(), BROADCAST_PORT);
    });
    nmeaSentenceSource->nmeaSentence.connect_to(nmeaSentenceReporter);
  }

  // DSC Input, GPS Output
  if (ENABLE_NMEA1) {
    swSerial[1] = new SoftwareSerial();
    swSerial[1]->begin(38400, SWSERIAL_8N1, NMEA1_RX, NMEA1_TX, false, bufCapacity, isrBufCapacity);
    auto *nmeaSentenceSource = new NmeaSentenceSource(swSerial[1]);
    auto nmeaSentenceReporter = new LambdaConsumer<String>([](String msg) {
      Serial.print(msg);
      udp.broadcastTo(msg.c_str(), BROADCAST_PORT);
    });
    nmeaSentenceSource->nmeaSentence.connect_to(nmeaSentenceReporter);
  }

  // Nexus FDX Input
  if (ENABLE_NMEA2) {
    swSerial[2] = new SoftwareSerial();
    swSerial[2]->begin(9600, SWSERIAL_8S1, NMEA2_RX, NMEA2_TX, false, bufCapacity, isrBufCapacity);
    auto *fdxSource = new FdxSource(swSerial[2]);
    auto nmeaSentenceReporter = new LambdaConsumer<String>([](String msg) {
      Serial.print(msg);
      //if (swSerial[1])
      //  swSerial[1]->print(msg);
      udp.broadcastTo(msg.c_str(), BROADCAST_PORT);
    });
    NmeaMessage* trueWindMessage = new NmeaMessage('T');
    fdxSource->fdxData.trueWind.direction.connect_to(new MovingAverage(10))->connect_to(trueWindMessage, 0);
    fdxSource->fdxData.trueWind.speed.connect_to(new MovingAverage(5))->connect_to(trueWindMessage, 1);
    trueWindMessage->connect_to(nmeaSentenceReporter);

    auto rawMessageReporter = new LambdaConsumer<String>([](String msg) {
      udp.broadcastTo(msg.c_str(), BROADCAST_PORT+1);
    });
    fdxSource->fdxData.rawMessage.connect_to(rawMessageReporter);
  }

  if (ENABLE_ELITE4HDI) {
    swSerial[3] = new SoftwareSerial();
    swSerial[3]->begin(38400, SWSERIAL_8N1, NMEA3_RX, NMEA3_TX, false, bufCapacity, isrBufCapacity);
    auto *nmeaSentenceSource = new NmeaSentenceSource(swSerial[3]);
    auto nmeaSentenceReporter = new LambdaConsumer<String>([](String msg) {
      Serial.print(msg);
      if (swSerial[1])
        swSerial[1]->print(msg);
      udp.broadcastTo(msg.c_str(), BROADCAST_PORT);
    });
    nmeaSentenceSource->nmeaSentence.connect_to(nmeaSentenceReporter);
  }

  if (ENABLE_GPS) {
    configureUbloxM8Gps(); 

    swSerial[7] = new SoftwareSerial();
    swSerial[7]->begin(9600, SWSERIAL_8N1, GPS_RX, GPS_TX, false, bufCapacity, isrBufCapacity);

    auto *nmeaSentenceSource = new NmeaSentenceSource(swSerial[7]);
    auto nmeaSentenceReporter = new LambdaConsumer<String>([](String msg) {
      Serial.print(msg);
      if (swSerial[1])
        swSerial[1]->print(msg);
      udp.broadcastTo(msg.c_str(), BROADCAST_PORT);
    });
    nmeaSentenceSource->nmeaSentence.connect_to(nmeaSentenceReporter);
  }
  Enable::enable_all();
}

ReactESP app(setupApp);

void configureUbloxM8Gps() {
  // Use HardwareSerial2 to configure GPS, since Baudrate of 115200 is
  // problamatic with SoftwareSerial and setting the baudrate is not working
  Serial2.begin(115200, SERIAL_8N1, GPS_RX, GPS_TX);
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

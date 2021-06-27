#include <Arduino.h>
#include <ESPmDNS.h>
#include <SoftwareSerial.h>
#include <WiFi.h>

#include "FdxSource.h"
#include "NmeaSentenceSource.h"
#include "TcpServer.h"
#include "UdpServer.h"
#include "icm20948.h"
#include "sensesp.h"
#include "system/lambda_consumer.h"
#include "transforms/NmeaMessage.h"
#include "transforms/NmeaMessage.t.hpp"
#include "transforms/lambda_transform.h"
#include "transforms/moving_average.h"
#include "wiring_helpers.h"

void configureUbloxM8Gps();

static const bool ENABLE_GPS = true;
static const bool ENABLE_NMEA0 = true;       // Radio: AIS Input
static const bool ENABLE_NMEA1 = true;       // Radio: DSC Input, GPS Output
static const bool ENABLE_NMEA2 = true;       // Nexus FDX
static const bool ENABLE_ELITE4HDI = false;  // GPS Input, AIS Output
static const bool ENABLE_ICM20948 = true;

const char *ssid = "Schmuddelwetter_24G";
const char *password = "9568164986244857";
const char *ssid_svala = "Svala";
const char *password_svala = "8641009916";
IPAddress local_IP(192, 168, 1, 1);
const uint16_t BROADCAST_PORT = 2000;
const uint16_t TCP_SERVER_PORT = 8375;

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

NetworkPublisher *networkPublisher;
NetworkPublisher *debugPublisher = new UdpServer(BROADCAST_PORT + 1);

SoftwareSerial *swSerial[8];
StringProducer *stringProducer;

void setupApp() {
  Serial.begin(115200);
  Serial.printf("\nHello, starting now..\n");

  for (int i = 0; i < 8; i++) {
    swSerial[i] = NULL;
  }

  Serial.printf("Connecting to Wifi Station with ssid '%s'.\n", ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("WiFi Station not found. Falling back to AP.");
    Serial.printf("Wifi AP with ssid '%s'.\n", ssid_svala);
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid_svala, password_svala);
    Serial.printf("Got IP %s\n", WiFi.softAPIP().toString().c_str());
  } else {
    Serial.println("WiFi connected to Station.");
    Serial.printf("Got IP %s\n", WiFi.localIP().toString().c_str());
  }

  if (!MDNS.begin("svala")) {
    Serial.println("Error starting mDNS");
    return;
  }

  // networkPublisher = new UdpServer(BROADCAST_PORT);
  networkPublisher = new TcpServer(TCP_SERVER_PORT);

  // swSerial.begin(9600, SWSERIAL_8S1, GPIO_NUM_21);
  // pinMode(GPIO_NUM_26, INPUT);
  // pinMode(GPIO_NUM_18, OUTPUT);

  auto nmeaSentenceReporter = new LambdaConsumer<String>([](String msg) {
    // Serial.print(msg);
    networkPublisher->send(msg.c_str());
  });

  auto rawMessageReporter = new LambdaConsumer<String>(
      [](String msg) { debugPublisher->send(msg.c_str()); });

  NmeaSentenceSource *nmea4;
  if (ENABLE_ELITE4HDI) {
    // Elite4HDI_TX
    swSerial[4] = new SoftwareSerial();
    swSerial[4]->begin(38400, SWSERIAL_8N1, NMEA4_RX, NMEA4_TX, false,
                       bufCapacity, isrBufCapacity);
    nmea4 = new NmeaSentenceSource(swSerial[4]);
    nmea4->connect_to(nmeaSentenceReporter);
  }

  // AIS input
  NmeaSentenceSource *nmea0;
  if (ENABLE_NMEA0) {
    swSerial[0] = new SoftwareSerial();
    swSerial[0]->begin(38400, SWSERIAL_8N1, NMEA0_RX, NMEA0_TX, false,
                       bufCapacity, isrBufCapacity);
    nmea0 = new NmeaSentenceSource(swSerial[0]);
    nmea0->connect_to(nmeaSentenceReporter);
  }

  // DSC Input, GPS Output
  NmeaSentenceSource *nmea1;
  if (ENABLE_NMEA1) {
    swSerial[1] = new SoftwareSerial();
    swSerial[1]->begin(38400, SWSERIAL_8N1, NMEA1_RX, NMEA1_TX, false,
                       bufCapacity, isrBufCapacity);
    nmea1 = new NmeaSentenceSource(swSerial[1]);
    nmea1->connect_to(nmeaSentenceReporter);
  }

  // Nexus FDX Input
  FdxSource *fdx;
  if (ENABLE_NMEA2) {
    swSerial[2] = new SoftwareSerial();
    swSerial[2]->begin(9600, SWSERIAL_8S1, NMEA2_RX, NMEA2_TX, false,
                       bufCapacity, isrBufCapacity);
    fdx = new FdxSource(swSerial[2]);
    NmeaMessage<float> *relativeWindMessage =
        new NmeaMessage<float>(MessageType::NMEA_MWV_RELATIVE);
    fdx->data.apparantWind.angle.connect_to(new MovingAverage(10))
        ->connect_to(relativeWindMessage, 0);
    fdx->data.apparantWind.speed.connect_to(new MovingAverage(5))
        ->connect_to(relativeWindMessage, 1);
    relativeWindMessage->connect_to(nmeaSentenceReporter);

    NmeaMessage<float> *trueWindMessage =
        new NmeaMessage<float>(MessageType::NMEA_MWV_TRUE);
    fdx->data.trueWind.angle.connect_to(new MovingAverage(10))
        ->connect_to(trueWindMessage, 0);
    fdx->data.trueWind.speed.connect_to(new MovingAverage(5))
        ->connect_to(trueWindMessage, 1);
    trueWindMessage->connect_to(nmeaSentenceReporter);

    fdx->data.temperature
        .connect_to(new NmeaMessage<float>(MessageType::NMEA_MTW))
        ->connect_to(nmeaSentenceReporter);

    fdx->data.voltage
        .connect_to(new NmeaMessage<float>(MessageType::NMEA_XDR_VOLTAGE))
        ->connect_to(nmeaSentenceReporter);

    fdx->data.signalStrength
        .connect_to(
            new NmeaMessage<float>(MessageType::NMEA_XDR_SIGNALSTRENGTH))
        ->connect_to(nmeaSentenceReporter);

    fdx->data.depth.connect_to(new MovingAverage(40))
        ->connect_to(new NmeaMessage<float>(MessageType::NMEA_DPT))
        ->connect_to(nmeaSentenceReporter);

    fdx->data.rawMessage.connect_to(rawMessageReporter);
  }

  NmeaSentenceSource *nmea3;
  if (ENABLE_ELITE4HDI) {
    swSerial[3] = new SoftwareSerial();
    swSerial[3]->begin(38400, SWSERIAL_8N1, NMEA3_RX, NMEA3_TX, false,
                       bufCapacity, isrBufCapacity);
    nmea3 = new NmeaSentenceSource(swSerial[3]);
    if (nmea1) nmea3->connect_to(nmea1);
    nmea3->connect_to(nmeaSentenceReporter);
  }

  NmeaSentenceSource *gps;
  if (ENABLE_GPS) {
    configureUbloxM8Gps();

    swSerial[7] = new SoftwareSerial();
    swSerial[7]->begin(9600, SWSERIAL_8N1, GPS_RX, GPS_TX, false, bufCapacity,
                       isrBufCapacity);

    gps = new NmeaSentenceSource(swSerial[7]);
    if (nmea1) gps->connect_to(nmea1);
    gps->connect_to(nmeaSentenceReporter);
  }

  Icm20948 *icm;
  if (ENABLE_ICM20948) {
    icm = new Icm20948();
    icm->data.pitch
        .connect_to(new NmeaMessage<double>(MessageType::NMEA_XDR_PITCH))
        ->connect_to(nmeaSentenceReporter);
    icm->data.roll
        .connect_to(new NmeaMessage<double>(MessageType::NMEA_XDR_ROLL))
        ->connect_to(nmeaSentenceReporter);
    icm->data.yaw.connect_to(new NmeaMessage<double>(MessageType::NMEA_HDM))
        ->connect_to(nmeaSentenceReporter);
  }

  /*
  stringProducer = new StringProducer();
  stringProducer->connect_to(nmeaSentenceReporter);
  app.onRepeat(1000, []() {
    char buf[100];
    //sprintf(buf, "Ping %lu\n", millis());
    sprintf(buf, "$GPGLL,3751.65,S,14507.36,E*77\r\n");
    stringProducer->emit(buf);
  });
  */

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

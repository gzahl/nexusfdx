#include <Arduino.h>
#include <AsyncUDP.h>
#include <SoftwareSerial.h>
#include <WiFi.h>

#include "sensesp.h"
#include "wiring_helpers.h"
#include "system/lambda_consumer.h"

#include "SerialFdxListenerTask.h"
#include "SerialNmeaListenerTask.h"

#include "NmeaSentenceParser.h"

void configureUbloxM8Gps();
static char *getLine(std::vector<uint8_t> buffer);

static const bool ENABLE_GPS = true;
static const bool ENABLE_NMEA0 = false;     // Radio: AIS Input
static const bool ENABLE_NMEA1 = false;     // Radio: DSC Input, GPS Output
static const bool ENABLE_NMEA2 = false;     // Nexus FDX
static const bool ENABLE_ELITE4HDI = false; // GPS Input, AIS Output
static const bool ENABLE_WIFI = false;

const char *ssid = "Schmuddelwetter_24G";
const char *password = "9568164986244857";
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
static const gpio_num_t GPS_RX = GPIO_NUM_23;
static const gpio_num_t GPS_TX = GPIO_NUM_19;

AsyncUDP udp;

SoftwareSerial *swSerial[8];

ReactESP app([]() {
  Serial.begin(115200);
  Serial.printf("\nHello, starting now..\n");

  for (int i = 0; i < 8; i++)
  {
    swSerial[i] = NULL;
  }

  if (ENABLE_WIFI)
  {
    Serial.printf("Connecting to Wifi with ssid '%s'.\n", ssid);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    if (WiFi.waitForConnectResult() != WL_CONNECTED)
    {
      Serial.println("WiFi Failed");
      while (1)
      {
        delay(1000);
      }
    }
    else
    {
      Serial.println("WiFi connected.");
    }
  }

  // swSerial.begin(9600, SWSERIAL_8S1, GPIO_NUM_21);
  // pinMode(GPIO_NUM_26, INPUT);
  // pinMode(GPIO_NUM_18, OUTPUT);

  if (ENABLE_ELITE4HDI)
  {
    // Elite4HDI_TX
    swSerial[4] = new SoftwareSerial();
    swSerial[4]->begin(38400, SWSERIAL_8N1, NMEA4_RX, NMEA4_TX);
  }

  // AIS input
  if (ENABLE_NMEA0)
  {
    swSerial[0] = new SoftwareSerial();
    swSerial[0]->begin(38400, SWSERIAL_8N1, NMEA0_RX, NMEA0_TX);
    new SerialNmeaListenerTask(
        "NMEA0_AIS", swSerial[0], [](std::vector<uint8_t> &msg) {
          Serial.print(getLine(msg));
          if (swSerial[4])
            swSerial[4]->print(getLine(msg));
          if (ENABLE_WIFI)
            udp.broadcastTo(getLine(msg), BROADCAST_PORT);
        });
  }

  // DSC Input, GPS Output
  if (ENABLE_NMEA1)
  {
    swSerial[1] = new SoftwareSerial();
    swSerial[1]->begin(38400, SWSERIAL_8N1, NMEA1_RX, NMEA1_TX);
    new SerialNmeaListenerTask(
        "NMEA1_DSCGPS", swSerial[1], [](std::vector<uint8_t> &msg) {
          Serial.print(getLine(msg));
          if (ENABLE_WIFI)
            udp.broadcastTo(getLine(msg), BROADCAST_PORT);
        });
  }

  // Nexus FDX Input
  if (ENABLE_NMEA2)
  {
    swSerial[2] = new SoftwareSerial();
    swSerial[2]->begin(9600, SWSERIAL_8S1, NMEA2_RX, NMEA2_TX);
    new SerialFdxListenerTask("NMEA2_NexusFDX", swSerial[2],
                              [](std::vector<uint8_t> &msg) {});
  }

  if (ENABLE_ELITE4HDI)
  {
    swSerial[3] = new SoftwareSerial();
    swSerial[3]->begin(38400, SWSERIAL_8N1, NMEA3_RX, NMEA3_TX);
    new SerialNmeaListenerTask(
        "NMEA3_ELITE4HDI_RX", swSerial[3], [](std::vector<uint8_t> &msg) {
          Serial.print(getLine(msg));
          if (swSerial[1])
            swSerial[1]->print(getLine(msg));
          if (ENABLE_WIFI)
            udp.broadcastTo(getLine(msg), BROADCAST_PORT);
        });
  }

  /*
  Serial2.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);
  app.onAvailable(Serial2, [](){
    while (Serial2.available()) {
      Serial.print((char) Serial2.read());
    }

  });
  
  while(true) {
    while(Serial2.available()) {
      char c = Serial2.read();
      Serial.print(c);
    }
  }*/

/*
  swSerial[7] = new SoftwareSerial();
  swSerial[7]->begin(9600, SWSERIAL_8N1, 23, 19, false, 64, 20);
  while(true) {
    while(swSerial[7]->available()) {
      char c = swSerial[7]->read();
      Serial.print(c);
    }
   //     vTaskDelay(1/ portTICK_PERIOD_MS);
  }*/

  if (ENABLE_GPS)
  {
    //Serial2.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);

    //configureUbloxM8Gps();
    
    swSerial[7] = new SoftwareSerial();
    swSerial[7]->begin(9600, SWSERIAL_8N1, GPS_RX, GPS_TX, false, 64, 20);
    if(swSerial[7]->isListening())
      Serial.println("SwSerial[7] working!");
      
    /*
    new SerialNmeaListenerTask(
        "GPS", swSerial[7], [](std::vector<uint8_t> &msg) {
          Serial.print(getLine(msg));
          if (swSerial[1])
            swSerial[1]->print(getLine(msg));
          if (ENABLE_WIFI)
            udp.broadcastTo(getLine(msg), BROADCAST_PORT);
        }, true);*/

    /*
    GPSInput* gpsInput = new GPSInput(&Serial2);

    auto gpsReporter = new LambdaConsumer<int>([](int numberOfSats){Serial.println("Number of sats: " + numberOfSats);});
    auto gnssQualityReporter = new LambdaConsumer<String>([](String gnssQuality){Serial.println("GNSS Quality: " + gnssQuality);});
    auto datetimeReporter = new LambdaConsumer<time_t>([](time_t time){Serial.println(time);});

    gpsInput->nmea_data_.num_satellites.connect_to(gpsReporter);
    gpsInput->nmea_data_.gnss_quality.connect_to(gnssQualityReporter);
    gpsInput->nmea_data_.datetime.connect_to(datetimeReporter);*/

    auto *nmeaSentenceParser = new NmeaSentenceParser(swSerial[7]);
    auto nmeaSentenceReporter = new LambdaConsumer<String>([](String msg) { Serial.println("Msg: " + msg); });
    nmeaSentenceParser->nmeaSentence.connect_to(nmeaSentenceReporter);
  }
  Enable::enable_all();
});

void configureUbloxM8Gps()
{
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

static char *getLine(std::vector<uint8_t> buffer)
{
  return reinterpret_cast<char *>(buffer.data());
}
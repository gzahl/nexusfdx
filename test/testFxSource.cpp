#include "FdxSource.h"
#include <Arduino.h>
#include <unity.h>

void testSentenceMWV() {
  auto fdxSource = new FdxSource(NULL);
  char actual[100];
  char expected[100];

  strcpy(actual, fdxSource->writeSentenceMWV(0.0, 0.0).c_str());
  TEST_ASSERT_EQUAL_STRING(expected, actual);
}

void setup() {
  // NOTE!!! Wait for >2 secs
  // if board doesn't support software reset via Serial.DTR/RTS
  delay(2000);

  UNITY_BEGIN(); // IMPORTANT LINE!
  RUN_TEST(testSentenceMWV);
}

uint8_t i = 0;
uint8_t max_blinks = 5;

void loop() {
  UNITY_END(); // stop unit testing
}
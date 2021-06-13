#include <FdxParser.h>
#include <string.h>
#include <unity.h>

unsigned char hexToBytes(const char* str, unsigned char* buf) {
  const char* pos = str;
  memset(buf, 0, sizeof buf / sizeof *buf);
  unsigned char len = (strlen(str) + 1) / 3;
  // printf("%s %i\n", "len:", len);

  /* WARNING: no sanitization or error-checking whatsoever */
  for (size_t count = 0; count <= len; count++) {
    sscanf(pos, "%2hhX", &buf[count]);
    pos += 3;
  }
  return len;
}

void printBytes(unsigned char* buf, unsigned char len) {
  for (size_t count = 0; count < len; count++) printf("%02X ", buf[count]);
  printf("\n");
}

FdxParser parse(const char* str) {
  unsigned char buf[100];
  auto fdxParser = FdxParser();
  unsigned char len = hexToBytes(str, buf);
  printBytes(buf, len);
  fdxParser.parse(buf, len);
  return fdxParser;
}

void testApparentWind() {
  auto fdxParser = parse("12 03 03 59 1B 50");
  TEST_ASSERT_EQUAL(FdxType::APPARENT_WIND, fdxParser.data.type);
  TEST_ASSERT_EQUAL_FLOAT(38.11765, fdxParser.data.apparantWind.angle);
  TEST_ASSERT_EQUAL_FLOAT(16.6258, fdxParser.data.apparantWind.speed);
}

void testSignalStrength() {
  auto fdxParser = parse("70 89 AF 80 D6");
  TEST_ASSERT_EQUAL(FdxType::WIND_TRANSDUCER_SIGNAL, fdxParser.data.type);
  TEST_ASSERT_EQUAL_FLOAT(68.62746, fdxParser.data.signalStrength);
}

void testTrueWind() {
  auto fdxParser = parse("01 05 03 5B 1B 47");
  TEST_ASSERT_EQUAL(FdxType::TRUE_WIND, fdxParser.data.type);
  TEST_ASSERT_EQUAL_FLOAT(38.11765, fdxParser.data.trueWind.angle);
  TEST_ASSERT_EQUAL_FLOAT(16.6646, fdxParser.data.trueWind.speed);
}

void testTemperature() {
  auto fdxParser = parse("08 16 1E");
  TEST_ASSERT_EQUAL(FdxType::TEMPERATURE, fdxParser.data.type);
  TEST_ASSERT_EQUAL_FLOAT(22., fdxParser.data.temperature);
}

void testVoltage() {
  auto fdxParser = parse("09 7E 77");
  TEST_ASSERT_EQUAL(FdxType::VOLTAGE, fdxParser.data.type);
  TEST_ASSERT_EQUAL_FLOAT(12.6, fdxParser.data.voltage);
}

void testDepth() {
  auto fdxParser = parse("07 18 01 00 1E");
  TEST_ASSERT_EQUAL(FdxType::DEPTH, fdxParser.data.type);
  TEST_ASSERT_EQUAL_FLOAT(2.4, fdxParser.data.depth);
}

int main(int argc, char** argv) {
  UNITY_BEGIN();
  RUN_TEST(testApparentWind);
  RUN_TEST(testSignalStrength);
  RUN_TEST(testTrueWind);
  RUN_TEST(testTemperature);
  RUN_TEST(testVoltage);
  RUN_TEST(testDepth);
  UNITY_END();

  return 0;
}
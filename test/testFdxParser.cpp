#include <FdxParser.h>
#include <string.h>
#include <unity.h>

unsigned char hexToBytes(const char* str, unsigned char* buf) {
  const char* pos = str;
  memset(buf, 0, sizeof buf / sizeof *buf);
  unsigned char len = (strlen(str)+1)/3;
  //printf("%s %i\n", "len:", len);

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

void testWind() {
  auto fdxParser = parse("12 03 03 59 1B 50");
  TEST_ASSERT_EQUAL(WIND, fdxParser.data.type);
  TEST_ASSERT_EQUAL_FLOAT(38.11765, fdxParser.data.relativeWind.angle);
  TEST_ASSERT_EQUAL_FLOAT(16.6258, fdxParser.data.relativeWind.speed);
}

void testSignalStrength() {
  auto fdxParser = parse("70 89 AF 80 D6");
  TEST_ASSERT_EQUAL(WIND_TRANSDUCER_SIGNAL, fdxParser.data.type);
  TEST_ASSERT_EQUAL_FLOAT(68.62746, fdxParser.data.signalStrength);
}

void testOtherWind() {
  auto fdxParser = parse("01 05 03 5B 1B 47");
  TEST_ASSERT_EQUAL(OTHER_WIND, fdxParser.data.type);
  TEST_ASSERT_EQUAL_FLOAT(38.11765, fdxParser.data.otherWind.angle);
  TEST_ASSERT_EQUAL_FLOAT(16.6646, fdxParser.data.otherWind.speed);
}

int main(int argc, char** argv) {
  UNITY_BEGIN();
  RUN_TEST(testWind);
  RUN_TEST(testSignalStrength);
  RUN_TEST(testOtherWind);
  UNITY_END();

  return 0;
}
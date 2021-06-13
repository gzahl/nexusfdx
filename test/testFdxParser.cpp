#include <FdxParser.h>
#include <unity.h>

void hexToBytes(const char* str, unsigned char* buf) {
  const char* pos = str;

  /* WARNING: no sanitization or error-checking whatsoever */
  for (size_t count = 0; count < sizeof buf / sizeof *buf; count++) {
    sscanf(pos, "%2hhx", &buf[count]);
    pos += 3;
  }

  // printf("0x");
  // for(size_t count = 0; count < sizeof val/sizeof *val; count++)
  //    printf("%02x", val[count]);
  // printf("\n");
}

void testWind() {
  unsigned char buf[100];
  auto fdxParser = FdxParser();
  hexToBytes("AA", buf);
  fdxParser.parse(buf, sizeof buf / sizeof *buf);
  // auto fdxSource = new FdxSource(NULL);
  TEST_ASSERT_EQUAL(WIND, fdxParser.data.type);
}

int main(int argc, char** argv) {
  UNITY_BEGIN();
  RUN_TEST(testWind);
  UNITY_END();

  return 0;
}
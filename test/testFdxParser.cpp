#include <FdxParser.h>
#include <unity.h>

void hexToBytes(char* str, unsigned char* buf) {
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

void testSentenceMWV() {
  // auto fdxSource = new FdxSource(NULL);
  char actual[100];
  char expected[100];

  // strcpy(actual, fdxSource->writeSentenceMWV(0.0, 0.0).c_str());
  // TEST_ASSERT_EQUAL_STRING(expected, actual);
  TEST_ASSERT_EQUAL_STRING("Hallo", "Hallo");
}

void testFails() { TEST_ASSERT_EQUAL_STRING("Not", "Equal"); }

int main(int argc, char** argv) {
  UNITY_BEGIN();
  RUN_TEST(testSentenceMWV);
  RUN_TEST(testFails);
  UNITY_END();

  return 0;
}
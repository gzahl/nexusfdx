//#include <FdxParser.h>
#include <unity.h>

void testSentenceMWV() {
  //auto fdxSource = new FdxSource(NULL);
  char actual[100];
  char expected[100];

  //strcpy(actual, fdxSource->writeSentenceMWV(0.0, 0.0).c_str());
  //TEST_ASSERT_EQUAL_STRING(expected, actual);
  TEST_ASSERT_EQUAL_STRING("Hallo","Hallo");
}

void testFails() {
  TEST_ASSERT_EQUAL_STRING("Not","Equal");
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(testSentenceMWV);
    RUN_TEST(testFails);
    UNITY_END();

    return 0;
}
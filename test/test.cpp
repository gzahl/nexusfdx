#include <unity.h>
#include "testFdxParser.hpp"
#include "testPwrUtility.hpp"

int main(int argc, char** argv) {
  UNITY_BEGIN();

  RUN_TEST(testApparentWind);
  RUN_TEST(testSignalStrength);
  RUN_TEST(testTrueWind);
  RUN_TEST(testTemperature);
  RUN_TEST(testVoltage);
  RUN_TEST(testDepth);
  RUN_TEST(testSpeed);

  RUN_TEST(testPwrUtility);

  UNITY_END();

  return 0;
}
#include <unity.h>

#include "pwrUtility.hpp"

void testPwrUtility() {
  TEST_ASSERT_EQUAL_DOUBLE(pow(2., 30.), pwrtwo(30));
  TEST_ASSERT_EQUAL_DOUBLE(pow(2., 15.), pwrtwo(15));
}

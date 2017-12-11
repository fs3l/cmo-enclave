#include "./helper.h"

#include "utils.h"

BOOST_AUTO_TEST_CASE(cmove_test)
{
  int32_t a = 42;
  int32_t b = 0;
  cmove_int32(false, &a, &b);
  BOOST_CHECK(a == 42);
  BOOST_CHECK(b == 0);
  cmove_int32(true, &a, &b);
  BOOST_CHECK(a == 42);
  BOOST_CHECK(b == 42);
}

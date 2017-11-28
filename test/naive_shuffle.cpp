#include "./helper.h"
#include "algo.h"
#include "cmo.h"

BOOST_AUTO_TEST_CASE(naive_shuffle_test)
{
  int32_t len = 100;
  int32_t* input = gen_random_sequence(len);
  int32_t* output = new int32_t[len];
  naive_shuffle(input, input, output, len);
  for (int32_t i = 0; i < len; ++i) BOOST_CHECK(output[i] == i);
  delete[] input;
  delete[] output;
}

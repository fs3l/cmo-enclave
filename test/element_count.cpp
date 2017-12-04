#include "./helper.h"

#include "algo.h"
#include "cmo.h"

BOOST_AUTO_TEST_CASE(naive_shuffle_test)
{
  int32_t len = 1000;
  int32_t N = 10;
  int32_t* input = new int32_t[len];
  count_result_t* output = new count_result_t[N];
  for (int32_t i = 0; i < len; ++i) input[i] = random_int32() % N;
  element_count(input, len, N, output);
  for (int32_t i = 0; i < N; ++i) {
    count_result_t r = output[i];
    int32_t real = 0;
    for (int32_t j = 0; j < len; ++j)
      if (input[j] == r.element) ++real;
    BOOST_CHECK(r.count == real);
  }

  delete[] input;
  delete[] output;
}

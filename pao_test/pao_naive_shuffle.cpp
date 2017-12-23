#include "./helper.h"

#ifdef SGX_APP
#include "./sgx_helper.h"

#include "pao_algo_u.h"
#else
#include "pao_algo.h"
#endif
#include <stdio.h>

BOOST_AUTO_TEST_CASE(pao_naive_shuffle_test)
{
  int32_t len = 1000;
  int32_t* input = gen_random_sequence(len);
  int32_t* output = new int32_t[len];
#ifdef SGX_APP
  ecall_naive_shuffle(global_eid, input, input, output, len);
#else
  pao_naive_shuffle(input, input, output, len);
#endif
  for (int32_t i = 0; i < len; ++i) BOOST_CHECK(output[i] == i);
  delete[] input;
  delete[] output;
}

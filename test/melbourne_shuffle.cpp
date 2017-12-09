#include "./helper.h"

#ifdef SGX_APP
#include "./sgx_helper.h"

#include "algo_u.h"
#else
#include "algo.h"
#endif

BOOST_AUTO_TEST_CASE(melbourne_shuffle_test)
{
  int32_t len = 100;
  int32_t* input = gen_random_sequence(len);
  int32_t* output = new int32_t[len];
#ifdef SGX_APP
  ecall_melbourne_shuffle(global_eid, input, input, output, len, 1);
#else
  melbourne_shuffle(input, input, output, len, 1);
#endif
  for (int32_t i = 0; i < len; ++i) BOOST_CHECK(output[i] == i);
  delete[] input;
  delete[] output;
}

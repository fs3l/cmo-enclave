#include "./helper.h"

#ifdef SGX_APP
#include "./sgx_helper.h"

#include "algo_u.h"
#else
#include "algo.h"
#endif

BOOST_AUTO_TEST_CASE(cache_shuffle_test)
{
  int32_t len = 1000;
  int32_t* input = gen_random_sequence(len);
  int32_t* output = new int32_t[len];
#ifdef SGX_APP
  ecall_cache_shuffle(global_eid, input, input, output, len, 1.0, 1000);
#else
  cache_shuffle(input, input, output, len, 1.0, 1000);
#endif
  for (int32_t i = 0; i < len; ++i) BOOST_CHECK(output[i] == i);
  delete[] input;
  delete[] output;
}

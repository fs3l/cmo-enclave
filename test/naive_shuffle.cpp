#include "./helper.h"

#ifdef SGX_APP
#include "./sgx_helper.h"

#include "algo_u.h"

sgx_enclave_id_t global_eid = 0;
#else
#include "algo.h"
#endif

BOOST_AUTO_TEST_CASE(naive_shuffle_test)
{
  int32_t len = 1000;
  int32_t* input = gen_random_sequence(len);
  int32_t* output = new int32_t[len];
#ifdef SGX_APP
  ecall_naive_shuffle(global_eid, input, input, output, len);
#else
  naive_shuffle(input, input, output, len);
#endif
  for (int32_t i = 0; i < len; ++i) BOOST_CHECK(output[i] == i);
  delete[] input;
  delete[] output;
}

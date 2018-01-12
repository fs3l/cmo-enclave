#include "./helper.h"
#include <sys/time.h>
#ifdef SGX_APP
#include "./sgx_helper.h"

#include "algo_u.h"
#else
#include "algo.h"
#endif

BOOST_AUTO_TEST_CASE(melbourne_shuffle_test)
{
 // int32_t len = 32768;
 // int32_t len = 65536;
 // int32_t len = 131072;
 // int32_t len = 262144;
 // int32_t len = 524288;
 // int32_t len = 1048576;
 // int32_t len = 2197152;
 // int32_t len = 4194304;
 // int32_t len = 8388608;
  int32_t len = 16777216;
 // int32_t len = 33554432;
  
  //int32_t len = 100;
  int32_t* input = gen_random_sequence(len);
  int32_t* output = new int32_t[len];
  struct timeval begin,end;
  gettimeofday(&begin,NULL);
#ifdef SGX_APP
  ecall_melbourne_shuffle(global_eid, input, input, output, len, 1);
#else
  melbourne_shuffle(input, input, output, len, 1);
#endif
  gettimeofday(&end,NULL);
  printf("time spent=%ld\n",1000000*(end.tv_sec-begin.tv_sec)+end.tv_usec-begin.tv_usec);
  for (int32_t i = 0; i < len; ++i) BOOST_CHECK(output[i] == i);
  delete[] input;
  delete[] output;
}

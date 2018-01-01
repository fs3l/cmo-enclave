#include "./helper.h"

#include <sys/time.h>
#ifdef SGX_APP
#include "./sgx_helper.h"

#include "algo_u.h"
#else
#include "algo.h"
#endif

BOOST_AUTO_TEST_CASE(merge_sort_test)
{
  int32_t len = 262144;
  int32_t* input = gen_random_sequence(len);
  int32_t* output = new int32_t[len];
  struct timeval begin,end;
  gettimeofday(&begin,NULL);
#ifdef SGX_APP
  ecall_merge_sort(global_eid, input, output, len, 4);
#else
  merge_sort(input, output, len, 4);
#endif
  gettimeofday(&end,NULL);
  printf("time spent=%ld\n",1000000*(end.tv_sec-begin.tv_sec)+end.tv_usec-begin.tv_usec);
  for (int32_t i = 0; i < len; ++i) BOOST_CHECK(output[i] == i);
  delete[] input;
  delete[] output;
}

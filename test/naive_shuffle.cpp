#include "./helper.h"

#ifdef SGX_APP
#include "./sgx_helper.h"

#include "algo_u.h"
#else
#include "algo.h"
#endif
#include<sys/time.h>
extern int g_bktime;
BOOST_AUTO_TEST_CASE(naive_shuffle_test)
{
  int32_t len = 1000000;
  int32_t* input = gen_random_sequence(len);
  int32_t* output = new int32_t[len];
  struct timeval begin,end;
  gettimeofday(&begin,NULL);
#ifdef SGX_APP
  ecall_naive_shuffle(global_eid, input, input, output, len);
#else
  naive_shuffle(input, input, output, len);
#endif
// for(int i=0;i<len;i++)
//  output[input[i]] = input[i];
  gettimeofday(&end,NULL);
  printf("time spent=%ld\n",1000000*(end.tv_sec-begin.tv_sec)+end.tv_usec-begin.tv_usec);
  printf("bk time = %ld\n",g_bktime);
//  for (int32_t i = 0; i < len; ++i) BOOST_CHECK(output[i] == i);
  delete[] input;
  delete[] output;
}

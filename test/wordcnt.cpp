#include "./helper.h"
#include <sys/time.h>
#ifdef SGX_APP
#include "./sgx_helper.h"

#include "algo_u.h"
#else
#include "algo.h"
#endif

BOOST_AUTO_TEST_CASE(wordcnt_test)
{
  int32_t len = 100000;
  int32_t range = 26;
  int32_t* output = new int32_t[range];
  memset(output,0,4*range);
  int32_t* input = gen_random_alphabeta_sequence(len);
  struct timeval begin,end;
  gettimeofday(&begin,NULL);
  wordcnt(input,output,len);
  gettimeofday(&end,NULL);
  for(int i=0;i<26;i++)
    printf("%d\n",output[i]);
  printf("time spent=%ld\n",1000000*(end.tv_sec-begin.tv_sec)+end.tv_usec-begin.tv_usec);
}

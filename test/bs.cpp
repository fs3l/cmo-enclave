#include "./helper.h"
#include <sys/time.h>
#ifdef SGX_APP
#include "./sgx_helper.h"

#include "algo_u.h"
#else
#include "algo.h"
#endif

BOOST_AUTO_TEST_CASE(binary_search_test)
{
  int32_t len = 1024*1024;
  int32_t work_len =  1000;
  int32_t* index = new int32_t[len];
  for(int i=0;i<len;i++) index[i] = i;
  struct timeval begin,end;
  gettimeofday(&begin,NULL);
  binary_search(index,len,work_len);
  gettimeofday(&end,NULL);
  printf("time spent=%ld\n",1000000*(end.tv_sec-begin.tv_sec)+end.tv_usec-begin.tv_usec);
}

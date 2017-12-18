#include "./helper.h"

#include "algo.h"
#include "cmo.h"
#include <sys/time.h>
BOOST_AUTO_TEST_CASE(sorting_net_test)
{
  int32_t len = 16777216;
  int32_t* input = gen_random_sequence(len);
  int32_t* values = new int[len*VALUE_SIZE];
  struct timeval begin,end;
  gettimeofday(&begin,NULL);
  int64_t res = merger(len, 0, input,values);
  gettimeofday(&end,NULL);
  printf("time spent=%ld and res=%ld\n",1000000*(end.tv_sec-begin.tv_sec)+end.tv_usec-begin.tv_usec,res);
  for (int32_t i = 0; i < len; ++i) BOOST_CHECK(input[i] == i);
  delete[] input;
}

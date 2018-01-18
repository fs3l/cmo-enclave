#include "./helper.h"

#ifdef SGX_APP
#include "./sgx_helper.h"

#include "algo_u.h"
#else
#include "algo.h"
#endif
#include <map>
BOOST_AUTO_TEST_CASE(mapreduce_test)
{
  std::vector<kvpair_t> input_sorted(4);
  std::map<int,std::vector<int>> output;
  input_sorted[0].key = 11;
  input_sorted[0].value.push_back(0);
  input_sorted[1].key = 13;
  input_sorted[1].value.push_back(1);
  input_sorted[2].key = 15;
  input_sorted[2].value.push_back(2);
  input_sorted[3].key = 18;
  input_sorted[3].value.push_back(0);
  mapreduce_rt(input_sorted, 4, map_wc, reduce_wc,output,NULL);
  BOOST_CHECK(output.at(0)[0]==2);
  BOOST_CHECK(output.at(1)[0]==1);
  BOOST_CHECK(output.at(2)[0]==1);
}

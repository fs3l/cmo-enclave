#include "./helper.h"
#include "algo.h"
#include "cmo.h"

BOOST_AUTO_TEST_CASE(merge_sort_test)
{
  int32_t len = 100;
  vector<int32_t> in(gen_random_sequence(len));
  vector<int32_t> out(len);
  merge_sort(in.data(), out.data(), len);
  for (int32_t i = 0; i < len; ++i) BOOST_CHECK(out[i] == i);
}

#include "./helper.h"
#include "algo.h"
#include "cmo.h"

BOOST_AUTO_TEST_CASE(naive_shuffle_test)
{
  int32_t data[4] = {2, 3, 1, 0};
  int32_t perm[4] = {2, 3, 1, 0};
  int32_t out[4] = {0};

  naive_shuffle(data, perm, out, 4);

  for (int32_t i = 0; i < 4; ++i) BOOST_CHECK(out[i] == i);
}

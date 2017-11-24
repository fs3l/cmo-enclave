#include "./helper.h"
#include "cmo.h"

BOOST_AUTO_TEST_CASE(lib_simple_test)
{
  int32_t data[4] = {2, 3, 1, 0};
  int32_t perm[4] = {2, 3, 1, 0};
  int32_t out[4] = {0};

  CMO rt;

  ObIterator d = rt.init_ob_iterator(data, 4);
  ObIterator p = rt.init_ob_iterator(perm, 4);
  NobArray o = rt.init_nob_array(out, 4);

  rt.begin_leaky_sec();
  for (int32_t i = 0; i < 4; ++i) o.write_at(p.read_next(), d.read_next());
  rt.end_leaky_sec();

  for (int32_t i = 0; i < 4; ++i) BOOST_CHECK(out[i] == i);
}

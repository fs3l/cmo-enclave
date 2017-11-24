#include "./helper.h"
#include "cmo.h"

BOOST_AUTO_TEST_CASE(lib_simple_test)
{
  int32_t data[4] = {2, 3, 1, 0};
  int32_t perm[4] = {2, 3, 1, 0};
  int32_t out[4] = {0};

  CMO_p rt = init_cmo_runtime();
  ReadObIterator_p d = init_read_ob_iterator(rt, data, 4);
  ReadObIterator_p p = init_read_ob_iterator(rt, perm, 4);
  NobArray_p o = init_nob_array(rt, out, 4);

  begin_leaky_sec(rt);
  for (int32_t i = 0; i < 4; ++i)
    nob_write_at(o, ob_read_next(p), ob_read_next(d));
  end_leaky_sec(rt);

  free_cmo_runtime(rt);

  for (int32_t i = 0; i < 4; ++i) BOOST_CHECK(out[i] == i);
}

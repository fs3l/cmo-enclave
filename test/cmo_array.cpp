#include "./helper.h"

#include "cmo.h"
#include "cmo_array.h"

#define LEN 10

BOOST_AUTO_TEST_CASE(array_test)
{
  CMO_p rt = init_cmo_runtime();
  Array<int32_t> a(rt, LEN);

  begin_leaky_sec(rt);

  for (int32_t i = 0; i < a.size(); ++i) {
    a.write(i, &i);
  }

  for (int32_t i = 0; i < a.size(); ++i) {
    int32_t v;
    a.read(i, &v);
    BOOST_CHECK(v == i);
  }

  end_leaky_sec(rt);

  free_cmo_runtime(rt);
}

BOOST_AUTO_TEST_CASE(read_only_array_test)
{

  CMO_p rt = init_cmo_runtime();
  ROArray<int32_t> a(rt, LEN);

  for (int32_t i = 0; i < LEN; ++i) {
    a.write(i, &i);
  }

  begin_leaky_sec(rt);

  for (int32_t i = 0; i < a.size(); ++i) {
    int32_t v;
    a.read(i, &v);
    BOOST_CHECK(v == i);
  }

  end_leaky_sec(rt);

  free_cmo_runtime(rt);
}

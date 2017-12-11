#include "./helper.h"

#include "./cmo_helper.h"

#include "cmo.h"
#include "cmo_array.h"

const int32_t N = 10;

BOOST_AUTO_TEST_CASE(array_test)
{
  CMO_p rt = init_cmo_runtime();
  Array<int32_t> a(rt, N);

  begin_leaky_sec(rt);

  for (int32_t i = 0; i < a.size(); ++i) {
    a.write(i, &i);
  }

  for (int32_t i = 0; i < a.size(); ++i) {
    int32_t v;
    a.read(i, &v);
    CMO_BOOST_CHECK(rt, v == i);
  }

  end_leaky_sec(rt);

  for (int32_t i = 0; i < a.size(); ++i) {
    int32_t v;
    a.read_leaky(i, &v);
    BOOST_CHECK(v == i);
  }

  for (int32_t i = 0; i < a.size(); ++i) {
    int32_t v = i * 2;
    a.write_leaky(i, &v);
  }

  begin_leaky_sec(rt);

  for (int32_t i = 0; i < a.size(); ++i) {
    int32_t v;
    a.read(i, &v);
    CMO_BOOST_CHECK(rt, v == i * 2);
  }

  end_leaky_sec(rt);

  free_cmo_runtime(rt);
}

BOOST_AUTO_TEST_CASE(read_only_array_test)
{
  CMO_p rt = init_cmo_runtime();
  ROArray<int32_t> a(rt, N);

  for (int32_t i = 0; i < N; ++i) {
    a.write_leaky(i, &i);
  }

  begin_leaky_sec(rt);

  for (int32_t i = 0; i < a.size(); ++i) {
    int32_t v;
    a.read(i, &v);
    CMO_BOOST_CHECK(rt, v == i);
  }

  end_leaky_sec(rt);

  for (int32_t i = 0; i < a.size(); ++i) {
    int32_t v;
    a.read_leaky(i, &v);
    BOOST_CHECK(v == i);
  }

  free_cmo_runtime(rt);
}

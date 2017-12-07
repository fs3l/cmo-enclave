#include "./helper.h"

#include "./cmo_helper.h"

#include "cmo.h"
#include "cmo_readonly_set_map.h"

const int32_t N = 1000;

BOOST_AUTO_TEST_CASE(readonly_set_test)
{
  int32_t *data = new int32_t[N];
  for (int32_t i = 0; i < N; ++i) data[i] = i * 100;

  CMO_p rt = init_cmo_runtime();
  ROSet<int32_t> s(rt, N, data);

  begin_leaky_sec(rt);
  CMO_BOOST_CHECK(rt, s.find(0));
  CMO_BOOST_CHECK(rt, s.find(100));
  CMO_BOOST_CHECK(rt, s.find(1000));
  CMO_BOOST_CHECK(rt, s.find(100 * (N - 1)));
  CMO_BOOST_CHECK(rt, !s.find(-1));
  CMO_BOOST_CHECK(rt, !s.find(1));
  CMO_BOOST_CHECK(rt, !s.find(11));
  CMO_BOOST_CHECK(rt, !s.find(111));
  CMO_BOOST_CHECK(rt, !s.find(100 * N));
  end_leaky_sec(rt);
  free_cmo_runtime(rt);

  delete [] data;
}

BOOST_AUTO_TEST_CASE(readonly_map_test)
{
  int32_t *keys= new int32_t[N];
  int32_t *values= new int32_t[N];
  for (int32_t i = 0; i < N; ++i) {
    keys[i] = i * 100;
    values[i] = i;
  }

  CMO_p rt = init_cmo_runtime();
  ROMap<int32_t, int32_t> m(rt, N, keys, values);

  begin_leaky_sec(rt);
  int32_t idx, value;

  idx = m.find(0);
  CMO_BOOST_CHECK(rt, idx != -1);
  m.get_value(idx, &value);
  CMO_BOOST_CHECK(rt, value == 0);

  idx = m.find(100);
  CMO_BOOST_CHECK(rt, idx != -1);
  m.get_value(idx, &value);
  CMO_BOOST_CHECK(rt, value == 1);

  idx = m.find(1000);
  CMO_BOOST_CHECK(rt, idx != -1);
  m.get_value(idx, &value);
  CMO_BOOST_CHECK(rt, value == 10);

  idx = m.find(100 * (N - 1));
  CMO_BOOST_CHECK(rt, idx != -1);
  m.get_value(idx, &value);
  CMO_BOOST_CHECK(rt, value == N - 1);

  CMO_BOOST_CHECK(rt, m.find(-1) == -1);
  CMO_BOOST_CHECK(rt, m.find(1) == -1);
  CMO_BOOST_CHECK(rt, m.find(11) == -1);
  CMO_BOOST_CHECK(rt, m.find(111) == -1);
  CMO_BOOST_CHECK(rt, m.find(100 * N) == -1);
  end_leaky_sec(rt);
  free_cmo_runtime(rt);

  delete [] keys;
  delete [] values;
}


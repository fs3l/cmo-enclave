#include "./helper.h"

#include "./cmo_helper.h"

#include "cmo.h"
#include "cmo_avltree.h"

const int32_t N = 100;

BOOST_AUTO_TEST_CASE(avl_set_test)
{
  CMO_p rt = init_cmo_runtime();

  AVLSet<int32_t> s(rt, N);

  begin_leaky_sec(rt);
  CMO_BOOST_CHECK(rt, !s.find(1));
  for (int32_t i = 0; i < N; ++i) s.insert(i);
  for (int32_t i = 0; i < N; ++i) CMO_BOOST_CHECK(rt, s.find(i));
  for (int32_t i = 0; i < N; ++i) {
    if (i % 2 == 0) s.remove(i);
  }
  for (int32_t i = 0; i < N; ++i) {
    if (i % 2 == 0)
      CMO_BOOST_CHECK(rt, !s.find(i));
    else
      CMO_BOOST_CHECK(rt, s.find(i));
  }
  for (int32_t i = 0; i < N; ++i) s.remove(i);
  for (int32_t i = 0; i < N; ++i) CMO_BOOST_CHECK(rt, !s.find(i));
  end_leaky_sec(rt);
  free_cmo_runtime(rt);
}

BOOST_AUTO_TEST_CASE(avl_map_test)
{
  CMO_p rt = init_cmo_runtime();

  AVLMap<int32_t, int32_t> m(rt, N);
  int32_t addr, value;

  begin_leaky_sec(rt);
  for (int32_t i = 0; i < N; ++i) m.insert(i, i * 10);
  for (int32_t i = 0; i < N; ++i) {
    addr = m.find(i);
    CMO_BOOST_CHECK(rt, addr != -1);
    m.get_value(addr, &value);
    CMO_BOOST_CHECK(rt, value == i * 10);
  }
  for (int32_t i = 0; i < N; ++i) {
    if (i % 2 == 0)
      m.remove(i);
    else {
      addr = m.find(i);
      m.set_value(addr, i * 100);
    }
  }
  for (int32_t i = 0; i < N; ++i) {
    if (i % 2 == 0) {
      CMO_BOOST_CHECK(rt, m.find(i) == 0);
    } else {
      addr = m.find(i);
      CMO_BOOST_CHECK(rt, addr != 0);
      m.get_value(addr, &value);
      CMO_BOOST_CHECK(rt, value == i * 100);
    }
  }
  for (int32_t i = 0; i < N; ++i) m.remove(i);
  for (int32_t i = 0; i < N; ++i) CMO_BOOST_CHECK(rt, m.find(i) == 0);
  end_leaky_sec(rt);

  free_cmo_runtime(rt);
}

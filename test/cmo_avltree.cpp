#include "./helper.h"

#include "cmo.h"
#include "cmo_avltree.h"

BOOST_AUTO_TEST_CASE(avl_set_test)
{
  const int32_t N = 100;
  CMO_p rt = init_cmo_runtime();

  AVLSet<int32_t> s(rt, N);

  begin_leaky_sec(rt);
  BOOST_CHECK(!s.find(1));
  for (int32_t i = 0; i < N; ++i) s.insert(i);
  for (int32_t i = 0; i < N; ++i) BOOST_CHECK(s.find(i));
  for (int32_t i = 0; i < N; ++i) {
    if (i % 2 == 0) s.remove(i);
  }
  for (int32_t i = 0; i < N; ++i) {
    if (i % 2 == 0)
      BOOST_CHECK(!s.find(i));
    else
      BOOST_CHECK(s.find(i));
  }
  for (int32_t i = 0; i < N; ++i) s.remove(i);
  for (int32_t i = 0; i < N; ++i) BOOST_CHECK(!s.find(i));
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
    BOOST_CHECK(addr != -1);
    m.get_value(addr, &value);
    BOOST_CHECK(value == i * 10);
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
      BOOST_CHECK(m.find(i) == 0);
    } else {
      addr = m.find(i);
      BOOST_CHECK(addr != 0);
      m.get_value(addr, &value);
      BOOST_CHECK(value == i * 100);
    }
  }
  for (int32_t i = 0; i < N; ++i) m.remove(i);
  for (int32_t i = 0; i < N; ++i) BOOST_CHECK(m.find(i) == 0);
  end_leaky_sec(rt);

  free_cmo_runtime(rt);
}

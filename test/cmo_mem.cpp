#include "./helper.h"

#include "cmo.h"
#include "cmo_mem.h"

struct element {
  int16_t a;
  int16_t b;
  int16_t c;
};
typedef struct element element_t;
typedef struct element* element_p;

element_t random_element()
{
  element_t r;
  r.a = random_int32() % INT16_MAX;
  r.b = random_int32() % INT16_MAX;
  r.c = random_int32() % INT16_MAX;
  return r;
}

bool operator==(const element_t& lhs, const element_t& rhs)
{
  return lhs.a == rhs.a && lhs.b == rhs.b && lhs.c == rhs.c;
}

BOOST_AUTO_TEST_CASE(mem_test)
{
  int32_t size = calc_element_block_size<element_t>();
  int32_t* data = new int32_t[size];
  element_t a, b, c, d;
  a = random_element();
  write_element<element_t>(data, &a);
  read_element<element_t>(data, &b);
  BOOST_CHECK(a == b);

  CMO_p rt = init_cmo_runtime();
  NobArray_p nob = init_nob_array(rt, data, size);
  ReadNobArray_p r_nob = init_read_nob_array(rt, data, size);

  begin_leaky_sec(rt);
  read_element(nob, 0, &c);
  BOOST_CHECK(a == c);
  read_element(r_nob, 0, &d);
  BOOST_CHECK(a == d);
  a = random_element();
  write_element(nob, 0, &a);
  end_leaky_sec(rt);

  read_element<element_t>(data, &b);
  BOOST_CHECK(a == b);

  free_cmo_runtime(rt);
  delete[] data;
}

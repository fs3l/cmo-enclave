#include "./helper.h"

#include "./cmo_helper.h"

#include "cmo.h"
#include "cmo_mempool.h"

#define LEN 100

BOOST_AUTO_TEST_CASE(mempool_test)
{
  CMO_p rt = init_cmo_runtime();

  MemoryPool<int32_t> pool(rt, LEN);

  begin_leaky_sec(rt);
  CMO_BOOST_CHECK(rt, pool.empty());
  CMO_BOOST_CHECK(rt, !pool.full());

  int32_t addrs[LEN] = {-1};

  for (int32_t i = 0; i < LEN; ++i) {
    int32_t addr = pool.alloc_block();
    pool.write_block(addr, &i);
    addrs[i] = addr;
  }

  CMO_BOOST_CHECK(rt, !pool.empty());
  CMO_BOOST_CHECK(rt, pool.full());

  for (int32_t i = 0; i < LEN; ++i) {
    int32_t data;
    pool.read_block(addrs[i], &data);
    CMO_BOOST_CHECK(rt, data == i);
  }

  for (int32_t i = 0; i < LEN; ++i) {
    if (rand() % 2 == 0) {
      pool.free_block(addrs[i]);
      addrs[i] = -1;
    }
  }

  for (int32_t i = 0; i < LEN; ++i) {
    if (addrs[i] != -1) {
      int32_t data;
      pool.read_block(addrs[i], &data);
      CMO_BOOST_CHECK(rt, data == i);
      pool.free_block(addrs[i]);
      addrs[i] = -1;
    }
  }

  CMO_BOOST_CHECK(rt, pool.empty());
  CMO_BOOST_CHECK(rt, !pool.full());

  end_leaky_sec(rt);

  free_cmo_runtime(rt);
}

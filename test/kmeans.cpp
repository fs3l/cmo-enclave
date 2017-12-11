#include "./helper.h"

#ifdef SGX_APP
#include "./sgx_helper.h"

#include "algo_u.h"
#else
#include "algo.h"
#endif

BOOST_AUTO_TEST_CASE(kmeans_test)
{
  int32_t len = 100000;
  int32_t k = 5;
  int32_t* x_in = new int32_t[len];
  int32_t* y_in = new int32_t[len];
  int32_t* output = new int32_t[len];

  for (int32_t i = 0; i < len; ++i) {
    x_in[i] = (i % k) * 10000 + random_int32() % 10;
    y_in[i] = (i % k) * 10000 + random_int32() % 10;
  }

#ifdef SGX_APP
  ecall_kmeans(global_eid, x_in, y_in, len, k, output);
#else
  kmeans(x_in, y_in, len, k, output);
#endif

  for (int32_t i = k; i < len; ++i) {
    BOOST_CHECK(output[i] == output[i % k]);
  }

  delete[] x_in;
  delete[] y_in;
  delete[] output;
}

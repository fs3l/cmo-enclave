#ifndef SGX_HELPER_H
#define SGX_HELPER_H

#ifdef SGX_APP

#include "app.h"

#include <boost/test/included/unit_test.hpp>

sgx_enclave_id_t global_eid = 0;

struct GlobalFixture {
  GlobalFixture()
  {
    if (initialize_enclave("../../lib/libalgo_enclave.so", "enclave.token",
                           &global_eid) < 0) {
      BOOST_FAIL("fail to initialize enclave");
    }
  }
  ~GlobalFixture() { sgx_destroy_enclave(global_eid); }
};

BOOST_GLOBAL_FIXTURE(GlobalFixture);

#endif

#endif

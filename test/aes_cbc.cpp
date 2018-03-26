#include "./helper.h"

#ifdef SGX_APP
#include "./sgx_helper.h"

#include "algo_u.h"
#else
#include "algo.h"
#endif

BOOST_AUTO_TEST_CASE(naive_shuffle_test)
{
   int size;
   printf("input data size:\n");
   scanf("%d",&size);

   if (size % 32 != 0) {printf("input size is not multiple of 32"); return;}
   #ifdef SGX_APP
    Aes_cbc(size);
   #else
    Aes_cbc(size);
   #endif
  //for (int32_t i = 0; i < len; ++i) BOOST_CHECK(output[i] == i);
  //delete[] input;
  //delete[] output;
}

#include "./helper.h"

#ifdef SGX_APP
#include "./sgx_helper.h"

#include "algo_u.h"
#else
#include "algo.h"
#endif

BOOST_AUTO_TEST_CASE(mapreduce_test)
{
   kvpair_p input_sorted = new kvpair_t[4];
   input_sorted[0].key = 11;
   input_sorted[0].value = 70;
   input_sorted[1].key = 13;
   input_sorted[1].value = 88;
   input_sorted[2].key = 15;
   input_sorted[2].value = 60;
   input_sorted[3].key = 18;
   input_sorted[3].value = 78;
   mapreduce_rt(input_sorted, 4, map_wc, reduce_wc);
   delete[] input_sorted;
}

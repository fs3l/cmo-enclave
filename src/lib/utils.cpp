#include "utils.h"

#include <stdlib.h>
#include <stdio.h>

#ifdef SGXAPI
#include <sgx_trts.h>
#endif

int32_t random_int32()
{
#ifdef SGXAPI
  int32_t result;
  sgx_read_rand((unsigned char*)&result, 4);
  return abs(result);
#else
  return rand() % INT32_MAX;
#endif
}

void fisher_yates_shuffle(int32_t* arr, int32_t len)
{
  for (int32_t i = 0; i < len - 1; ++i) {
    int32_t j = random_int32() % len;
    swap(&arr[i], &arr[j]);
  }
}

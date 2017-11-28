#include "utils.h"

#include <stdio.h>
#include <stdlib.h>

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

int32_t* gen_sequence(int32_t len, int32_t start_value)
{
  int32_t* result = new int32_t[len];
  int32_t i = 0;
  while (i < len) result[i++] = start_value++;
  return result;
}

int32_t* gen_random_sequence(int32_t len, int32_t start_value)
{
  int32_t* result = gen_sequence(len, start_value);
  fisher_yates_shuffle<int32_t>(result, len);
  return result;
}

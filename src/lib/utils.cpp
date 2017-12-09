#include "utils.h"

#ifdef SGX_ENCLAVE
#include <sgx_trts.h>
#else
#include <cstdlib>
#endif

#include <cstdarg>
#include <cstdio>

int32_t random_int32()
{
#ifdef SGX_ENCLAVE
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

void abort_message(const char* fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  print_message(fmt, args);
  va_end(args);
#ifdef SGX_ENCLAVE
  ocall_abort();
#else
  abort();
#endif
}

void print_message(const char* fmt, ...)
{
#ifdef SGX_ENCLAVE
  char buffer[512];
  va_list args;
  va_start(args, fmt);
  vsnprintf(buffer, 512, fmt, args);
  va_end(args);
  ocall_print(buffer);
#else
  va_list args;
  va_start(args, fmt);
  printf(fmt, args);
  va_end(args);
#endif
}

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
void cmove_int64(bool cond, const int64_t* src, int64_t* dst)
{
  // TODO:
  
  if (cond) {
    *dst = *src;
  }
}

void cmove_int32(bool cond, const int32_t* src, int32_t* dst)
{
  // TODO:
  
  if (cond) {
    *dst = *src;
  }
  
/*  
  __asm__(
  "movb %0, %%al\n\t"
  "mov %1, %%rsi\n\t"
  "mov %2, %%rdx\n\t"
  "mov (%%rsi), %%ebx\n\t"
  "mov (%%rdx), %%ecx\n\t"
  "test %%rax, %%rax\n\t"
  "cmovnz %%ebx, %%ecx\n\t"
  "mov %%ecx, (%%rdx)\n\t"
  : 
  : "r"(cond), "r"(src), "r"(dst)
  : "%rsi", "%rdx", "%rax", "%rbx", "%rcx");*/ 
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

int32_t* gen_random_alphabeta_sequence(int32_t len) {
  int32_t* result = new int32_t[len];
  int32_t i = 0;
  while (i<len) result[i++] = random_int32()%26;
  return result;
}

void abort_message(const char* fmt, ...)
{
#ifdef SGX_ENCLAVE
  char buffer[512] = {'\0'};
  va_list args;
  va_start(args, fmt);
  vsnprintf(buffer, 512, fmt, args);
  va_end(args);
  ocall_print(buffer);
  ocall_abort();
#else
  va_list args;
  va_start(args, fmt);
  vprintf(fmt, args);
  va_end(args);
  abort();
#endif
}

void print_message(const char* fmt, ...)
{
#ifdef SGX_ENCLAVE
  char buffer[512] = {'\0'};
  va_list args;
  va_start(args, fmt);
  vsnprintf(buffer, 512, fmt, args);
  va_end(args);
  ocall_print(buffer);
#else
  va_list args;
  va_start(args, fmt);
  vprintf(fmt, args);
  va_end(args);
#endif
}

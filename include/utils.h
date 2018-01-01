#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <cmath>

template <class T>
inline T min(const T& a, const T& b)
{
  return a < b ? a : b;
}

template <class T>
inline T max(const T& a, const T& b)
{
  return a > b ? a : b;
}

template <class T>
inline void swap(T* a, T* b)
{
  T tmp = *a;
  *a = *b;
  *b = tmp;
}

int32_t random_int32();
void cmove_int32(bool cond, const int32_t* src, int32_t* dest);
void cmove_int64(bool cond, const int64_t* src, int64_t* dest);

template <class T>
void fisher_yates_shuffle(T* arr, int32_t len)
{
  for (int32_t i = 0; i < len - 1; ++i) {
    int32_t j = random_int32() % len;
    swap(&arr[i], &arr[j]);
  }
}

int32_t* gen_sequence(int32_t len, int32_t start_value = 0);

int32_t* gen_random_sequence(int32_t len, int32_t start_value = 0);

void abort_message(const char* fmt, ...);

void print_message(const char* fmt, ...);

#endif

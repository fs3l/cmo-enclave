#ifndef UTILS_H
#define UTILS_H

#include <math.h>
#include <stdint.h>

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

inline void swap(int32_t* a, int32_t* b)
{
  int32_t tmp = *a;
  *b = *a;
  *a = tmp;
}

int32_t random_int32();

void fisher_yates_shuffle(int32_t* arr, int32_t len);

#endif

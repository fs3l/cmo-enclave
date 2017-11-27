#ifndef ALGO_H
#define ALGO_H

#include <stdint.h>

void naive_shuffle(const int32_t* arr_in, const int32_t* perm_in,
                   int32_t* arr_out, int32_t len);

void merge_sort(const int32_t* arr_in, int32_t* arr_out, int32_t len,
                int32_t blow_up_factor = 4);

#endif

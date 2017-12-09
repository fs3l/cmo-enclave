#ifndef ALGO_H
#define ALGO_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void naive_shuffle(const int32_t* arr_in, const int32_t* perm_in,
                   int32_t* arr_out, int32_t len);

void melbourne_shuffle(const int32_t* arr_in, const int32_t* perm_in,
                       int32_t* arr_out, int32_t len, int32_t blow_up_factor);

void cache_shuffle(const int32_t* arr_in, const int32_t* perm_in,
                   int32_t* arr_out, int32_t len, double epsilon,
                   int32_t mem_cap);

void merge_sort(const int32_t* arr_in, int32_t* arr_out, int32_t len,
                int32_t blow_up_factor);

struct count_result {
  int32_t element;
  int32_t count;
};
typedef struct count_result count_result_t;

void element_count(const int32_t* arr_in, int32_t len, int32_t uniq_elememts,
                   count_result_t* result);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

#include "algo.h"
#include "algo_t.h"

void ecall_naive_shuffle(const int32_t* arr_in, const int32_t* perm_in,
                         int32_t* arr_out, int32_t len)
{
  naive_shuffle(arr_in, perm_in, arr_out, len);
}

void ecall_melbourne_shuffle(const int32_t* arr_in, const int32_t* perm_in,
                             int32_t* arr_out, int32_t len,
                             int32_t blow_up_factor)
{
  melbourne_shuffle(arr_in, perm_in, arr_out, len, blow_up_factor);
}

void ecall_cache_shuffle(const int32_t* arr_in, const int32_t* perm_in,
                         int32_t* arr_out, int32_t len, double epsilon,
                         int32_t mem_cap)
{
  cache_shuffle(arr_in, perm_in, arr_out, len, epsilon, mem_cap);
}

void ecall_merge_sort(const int32_t* arr_in, int32_t* arr_out, int32_t len,
                      int32_t blow_up_factor)
{
  merge_sort(arr_in, arr_out, len, blow_up_factor);
}

void ecall_element_count(const int32_t* arr_in, int32_t len,
                         int32_t uniq_elements, struct count_result* result)
{
  element_count(arr_in, len, uniq_elements, result);
}

void ecall_kmeans(const int32_t* x_in, const int32_t* y_in, int32_t len,
                  int32_t k, int32_t* result)
{
  kmeans(x_in, y_in, len, k, result);
}

#include "algo.h"
#include "algo_t.h"

void ecall_naive_shuffle(const int32_t* arr_in, const int32_t* perm_in,
                         int32_t* arr_out, int32_t len)
{
  int32_t* _arr_in = new int32_t[len];
  int32_t* _perm_in = new int32_t[len];
  int32_t* _arr_out = new int32_t[len];
  for (int i = 0; i < len; ++i) {
    _arr_in[i] = arr_in[i];
    _perm_in[i] = perm_in[i];
  }
  naive_shuffle(_arr_in, _perm_in, _arr_out, len);
  for (int i = 0; i < len; ++i) {
    arr_out[i] = _arr_out[i];
  }
  delete[] _arr_in;
  delete[] _perm_in;
  delete[] _arr_out;
}

void ecall_melbourne_shuffle(const int32_t* arr_in, const int32_t* perm_in,
                             int32_t* arr_out, int32_t len,
                             int32_t blow_up_factor)
{
  int32_t* _arr_in = new int32_t[len];
  int32_t* _perm_in = new int32_t[len];
  int32_t* _arr_out = new int32_t[len];
  for (int i = 0; i < len; ++i) {
    _arr_in[i] = arr_in[i];
    _perm_in[i] = perm_in[i];
  }
  melbourne_shuffle(_arr_in, _perm_in, _arr_out, len, blow_up_factor);
  for (int i = 0; i < len; ++i) {
    arr_out[i] = _arr_out[i];
  }
  delete[] _arr_in;
  delete[] _perm_in;
  delete[] _arr_out;
}

void ecall_cache_shuffle(const int32_t* arr_in, const int32_t* perm_in,
                         int32_t* arr_out, int32_t len, double epsilon,
                         int32_t mem_cap)
{
  int32_t* _arr_in = new int32_t[len];
  int32_t* _perm_in = new int32_t[len];
  int32_t* _arr_out = new int32_t[len];
  for (int i = 0; i < len; ++i) {
    _arr_in[i] = arr_in[i];
    _perm_in[i] = perm_in[i];
  }
  cache_shuffle(_arr_in, _perm_in, _arr_out, len, epsilon, mem_cap);
  for (int i = 0; i < len; ++i) {
    arr_out[i] = _arr_out[i];
  }
  delete[] _arr_in;
  delete[] _perm_in;
  delete[] _arr_out;
}

void ecall_merge_sort(const int32_t* arr_in, int32_t* arr_out, int32_t len,
                      int32_t blow_up_factor)
{
  int32_t* _arr_in = new int32_t[len];
  int32_t* _arr_out = new int32_t[len];
  for (int i = 0; i < len; ++i) {
    _arr_in[i] = arr_in[i];
  }
  merge_sort(_arr_in, _arr_out, len, blow_up_factor);
  for (int i = 0; i < len; ++i) {
    arr_out[i] = _arr_out[i];
  }
  delete[] _arr_in;
  delete[] _arr_out;
}

void ecall_element_count(const int32_t* arr_in, int32_t len,
                         int32_t uniq_elements, struct count_result* result)
{
  int32_t* _arr_in = new int32_t[len];
  struct count_result* _result = new struct count_result[uniq_elements];
  for (int i = 0; i < len; ++i) {
    _arr_in[i] = arr_in[i];
  }
  element_count(_arr_in, len, uniq_elements, _result);
  for (int i = 0; i < uniq_elements; ++i) {
    result[i] = _result[i];
  }
  delete[] _arr_in;
  delete[] _result;
}

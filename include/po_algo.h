#ifndef PO_ALGO_H
#define PO_ALGO_H
#include <stdint.h>
void leakysec_distribute(int32_t* perm, int32_t* data, int32_t* perm_t, int32_t* data_t, int32_t sqrtN, int32_t max_elems, int32_t i);
void leakysec_cleanup(int32_t* perm_t, int32_t* data_t, int32_t* data, int32_t sqrtN, int32_t max_elems, int32_t i);
#endif

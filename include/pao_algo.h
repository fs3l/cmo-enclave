#ifndef PAOALGO_H
#define PAOALGO_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void pao_naive_shuffle(const int32_t* arr_in, const int32_t* perm_in,
                   int32_t* arr_out, int32_t len);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

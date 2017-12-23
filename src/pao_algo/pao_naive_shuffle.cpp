#include "pao_algo.h"
#include "pao.h"
void pao_naive_shuffle(const int32_t* arr_in, const int32_t* perm_in,
                   int32_t* arr_out, int32_t len)
{
  PAO_p rt = init_pao_runtime();
  ArrayRO_p d = init_array_ro(rt, arr_in, len);
  ArrayRO_p p = init_array_ro(rt, perm_in, len);
  ArrayRW_p o = init_array_rw(rt, arr_out, len);

  begin_leaky_sec(rt);
  for (int32_t i = 0; i < len; ++i)
    write_at(o, read_at(p,i), read_at(d,i));
  end_leaky_sec(rt);

  free_pao_runtime(rt);
}

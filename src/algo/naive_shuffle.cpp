#include "algo.h"
#include "cmo.h"

void naive_shuffle(const int32_t* arr_in, const int32_t* perm_in,
                   int32_t* arr_out, int32_t len)
{
  CMO_p rt = init_cmo_runtime();
  ReadObIterator_p d = init_read_ob_iterator(rt, arr_in, len);
  ReadObIterator_p p = init_read_ob_iterator(rt, perm_in, len);
  NobArray_p o = init_nob_array(rt, arr_out, len);

  begin_leaky_sec(rt);
  for (int32_t i = 0; i < len; ++i)
    nob_write_at(o, ob_read_next(p), ob_read_next(d));
  end_leaky_sec(rt);

  free_cmo_runtime(rt);
}

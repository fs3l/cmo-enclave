#include "algo.h"
#include "cmo.h"
#include "cmo_avltree.h"

void element_count(const int32_t* arr_in, int32_t len, int32_t uniq_elememts,
                   count_result_t* result)
{
  CMO_p rt = init_cmo_runtime();

  ReadObIterator_p ob_in = init_read_ob_iterator(rt, arr_in, len);
  AVLMap<int32_t, int32_t> counts(rt, uniq_elememts);

  int32_t i, element, count, addr, size;

  begin_leaky_sec(rt);
  for (i = 0; i < len; ++i) {
    element = ob_read_next(ob_in);
    addr = counts.find(element);
    if (addr == 0)
      counts.insert(element, 1);
    else {
      counts.get_value(addr, &count);
      counts.set_value(addr, count + 1);
    }
  }
  size = counts.size();
  if (size != uniq_elememts)
    cmo_abort(rt, "element_count: uniq_elememts mismatch");
  end_leaky_sec(rt);

  free_cmo_runtime(rt);
  rt = init_cmo_runtime();

  WriteObIterator_p ob_out =
      init_write_ob_iterator(rt, (int32_t*)result, uniq_elememts * 2);
  counts.reset_nob(rt);

  begin_leaky_sec(rt);
  addr = counts.begin_address();
  while (addr != 0) {
    counts.get_element(addr, &element, &count);
    ob_write_next(ob_out, element);
    ob_write_next(ob_out, count);
    addr = counts.next_address(addr);
  }
  end_leaky_sec(rt);

  free_cmo_runtime(rt);
}

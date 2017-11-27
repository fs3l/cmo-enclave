#include "algo.h"
#include "cmo.h"
#include "cmo_queue.h"

#include <math.h>

void merge_sort(const int32_t* arr_in, int32_t* arr_out, int32_t len,
                int32_t blow_up_factor)
{
  if (len == 1) {
    arr_out[0] = arr_in[0];
    return;
  }

  int32_t lhs_len = len / 2;
  int32_t rhs_len = len - lhs_len;
  int32_t* lhs_out = new int32_t[lhs_len];
  int32_t* rhs_out = new int32_t[rhs_len];
  merge_sort(arr_in, lhs_out, lhs_len, blow_up_factor);
  merge_sort(arr_in + lhs_len, rhs_out, rhs_len, blow_up_factor);

  int32_t s = blow_up_factor * (int)sqrt((double)len);
  int32_t half_s = s / 2;

  CMO_p rt = init_cmo_runtime();

  ReadObIterator_p lhs_out_ob = init_read_ob_iterator(rt, lhs_out, lhs_len);
  ReadObIterator_p rhs_out_ob = init_read_ob_iterator(rt, rhs_out, rhs_len);
  WriteObIterator_p arr_out_ob = init_write_ob_iterator(rt, arr_out, len);
  SimpleQueue<int32_t> lhs_q(rt, s), rhs_q(rt, s);

  int32_t i, j;
  int32_t lhs_val, rhs_val, out_val;

  begin_leaky_sec(rt);

  for (i = 0; i < half_s; ++i) {
    if (i < lhs_len) {
      if (lhs_q.full()) cmo_abort(rt, "merge_sort: queue is full");
      lhs_q.enqueue(ob_read_next(lhs_out_ob));
    }
    if (i < rhs_len) {
      if (rhs_q.full()) cmo_abort(rt, "merge_sort: queue is full");
      rhs_q.enqueue(ob_read_next(rhs_out_ob));
    }
  }

  for (; i < rhs_len + half_s; ++i) {
    if (i < lhs_len) {
      if (lhs_q.full()) cmo_abort(rt, "merge_sort: queue is full");
      lhs_q.enqueue(ob_read_next(lhs_out_ob));
    }
    if (i < rhs_len) {
      if (rhs_q.full()) cmo_abort(rt, "merge_sort: queue is full");
      rhs_q.enqueue(ob_read_next(rhs_out_ob));
    }

    for (j = 0; j < 2; ++j) {
      if (lhs_q.size() > 0 && rhs_q.size() > 0) {
        lhs_q.front(&lhs_val);
        rhs_q.front(&rhs_val);
        if (lhs_val < rhs_val) {
          out_val = lhs_val;
          lhs_q.dequeue();
        } else if (lhs_val == rhs_val) {
          out_val = lhs_val;
          if (lhs_q.size() > rhs_q.size())
            lhs_q.dequeue();
          else
            rhs_q.dequeue();
        } else {
          out_val = rhs_val;
          rhs_q.dequeue();
        }
        ob_write_next(arr_out_ob, out_val);
      } else {
        if (i < lhs_len || i < rhs_len)
          cmo_abort(rt, "merge_sort: queue is empty");
        goto done;
      }
    }
  }

  if (lhs_q.size() > 0 && rhs_q.size() > 0)
    cmo_abort(rt, "merge_sort: unprocessed data");
done:

  while (lhs_q.size() > 0) {
    lhs_q.front(&out_val);
    lhs_q.dequeue();
    ob_write_next(arr_out_ob, out_val);
  }

  while (rhs_q.size() > 0) {
    rhs_q.front(&out_val);
    rhs_q.dequeue();
    ob_write_next(arr_out_ob, out_val);
  }

  end_leaky_sec(rt);

  free_cmo_runtime(rt);
  delete[] lhs_out;
  delete[] rhs_out;
}

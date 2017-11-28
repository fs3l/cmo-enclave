#include "algo.h"
#include "cmo.h"
#include "cmo_queue.h"
#include "utils.h"

#include "./shuffle_bucket.h"

// S: number of elements to be read in one pass.
// num_of_bucket: number of buckets to be returned.
// mem_cap: max number of element can be stored in the nob.
static shuffle_bucket_p* _cache_shuffle_spray(const shuffle_bucket_p input,
                                              const int32_t S,
                                              const int32_t num_of_bucket,
                                              const int32_t mem_cap)
{
  const int32_t len = input->len;
  const int32_t begin_idx = input->begin_idx;
  const int32_t end_idx = input->end_idx;
  const int32_t bucket_len =
      max(ceil((double)(len) / S), ceil((double)(len) / num_of_bucket));
  const int32_t bucket_idx_len =
      round((double)(end_idx - begin_idx) / num_of_bucket);

  shuffle_bucket_p* buckets = init_empty_shuffle_buckets(
      num_of_bucket, bucket_len, begin_idx, end_idx, bucket_idx_len);

  const int32_t write_output_len = 2 * num_of_bucket;
  int32_t* write_output = new int32_t[write_output_len];

  CMO_p rt = init_cmo_runtime();

  ReadObIterator_p read_ob = shuffle_bucket_init_read_ob(input, rt);
  WriteObIterator_p write_ob =
      init_write_ob_iterator(rt, write_output, write_output_len);
  MultiQueue<shuffle_element_t> q(rt, mem_cap, num_of_bucket);

  int32_t i, read_idx, write_idx, bucket_idx, write_output_idx;
  shuffle_element_t e;

  read_idx = 0;
  write_idx = 0;
  while (read_idx < len) {
    begin_leaky_sec(rt);
    for (i = 0; i < S && read_idx < len; ++i) {
      e.value = ob_read_next(read_ob);
      e.perm = ob_read_next(read_ob);
      if (e.perm != -1) {
        bucket_idx = (e.perm - begin_idx) / bucket_idx_len;
        bucket_idx = min(bucket_idx, num_of_bucket - 1);
        if (q.full()) cmo_abort(rt, "cache_shuffle: queue full");
        q.push_back(bucket_idx, e);
      }
      ++read_idx;
    }

    for (bucket_idx = 0; bucket_idx < num_of_bucket; ++bucket_idx) {
      e.value = e.perm = -1;
      if (!q.empty(bucket_idx)) {
        q.front(bucket_idx, &e);
        q.pop_front(bucket_idx);
      }
      ob_write_next(write_ob, e.value);
      ob_write_next(write_ob, e.perm);
    }
    end_leaky_sec(rt);

    write_output_idx = 0;
    for (bucket_idx = 0; bucket_idx < num_of_bucket; ++bucket_idx) {
      buckets[bucket_idx]->data[write_idx * 2] =
          write_output[write_output_idx++];
      buckets[bucket_idx]->data[write_idx * 2 + 1] =
          write_output[write_output_idx++];
    }

    ++write_idx;
    reset_write_ob(write_ob);
  }

  free_cmo_runtime(rt);
  delete[] write_output;

  for (bucket_idx = 0; bucket_idx < num_of_bucket; ++bucket_idx) {
    for (write_idx = 0; write_idx < buckets[bucket_idx]->len; ++write_idx) {
      const int32_t this_bucket_len = buckets[bucket_idx]->len;

      rt = init_cmo_runtime();

      read_ob = shuffle_bucket_init_read_ob(buckets[bucket_idx], rt);
      write_ob = shuffle_bucket_init_write_ob(buckets[bucket_idx], rt);
      q.reset_nob(rt);

      begin_leaky_sec(rt);
      for (i = 0; i < this_bucket_len; ++i) {
        e.value = ob_read_next(read_ob);
        e.perm = ob_read_next(read_ob);
        if (e.perm == -1 && !q.empty(bucket_idx)) {
          q.front(bucket_idx, &e);
          q.pop_front(bucket_idx);
        }
        ob_write_next(write_ob, e.value);
        ob_write_next(write_ob, e.perm);
      }
      end_leaky_sec(rt);

      free_cmo_runtime(rt);
    }
  }

  return buckets;
}

void cache_shuffle(const int32_t* arr_in, const int32_t* perm_in,
                   int32_t* arr_out, int32_t len, double epsilon, int mem_cap)
{
  if (len == 1) {
    arr_out[0] = arr_in[0];
    return;
  }

  const int32_t S = max(1, (int32_t)log2((double)len));
  const int32_t Q = ceil((1 + epsilon) * S);
  shuffle_bucket_p input = init_shuffle_bucket(arr_in, perm_in, len, 0, len);

  // spray
  int32_t temp_len = find_suitable_partitions(len, Q);
  shuffle_bucket_p* temp = _cache_shuffle_spray(input, S, temp_len, mem_cap);

  // rspary
  bool done;
  int32_t new_temp_len;
  while (true) {
    done = true;
    new_temp_len = 0;
    for (int32_t i = 0; i < temp_len; ++i) {
      int32_t idx_len = temp[i]->end_idx - temp[i]->begin_idx;
      if (idx_len <= mem_cap) {
        new_temp_len += 1;
      } else {
        done = false;
        new_temp_len += find_suitable_partitions(idx_len, S);
      }
    }

    if (done) break;

    shuffle_bucket_p* new_temp = new shuffle_bucket_p[new_temp_len];

    for (int32_t i = 0, j = 0; i < temp_len; ++i) {
      int32_t idx_len = temp[i]->end_idx - temp[i]->begin_idx;
      if (idx_len <= mem_cap) {
        new_temp[j++] = temp[i];
      } else {
        randomize_shuffle_bucket(temp[i]);
        int32_t q = find_suitable_partitions(idx_len, S);
        shuffle_bucket_p* temp2 = _cache_shuffle_spray(temp[i], q, q, mem_cap);
        free_shuffle_bucket(temp[i]);
        for (int32_t k = 0; k < q; ++k) new_temp[j++] = temp2[k];
        delete[] temp2;
      }
    }
    delete[] temp;
    temp = new_temp;
    temp_len = new_temp_len;
  }

  for (int32_t bucket_idx = 0; bucket_idx < temp_len; ++bucket_idx) {
    shuffle_bucket_p bucket = temp[bucket_idx];
    int32_t bucket_len = bucket->len;
    int32_t begin_idx = bucket->begin_idx;
    int32_t end_idx = bucket->end_idx;

    CMO_p rt = init_cmo_runtime();

    ReadObIterator_p bucket_ob = shuffle_bucket_init_read_ob(bucket, rt);
    NobArray_p nob =
        init_nob_array(rt, arr_out + begin_idx, end_idx - begin_idx);

    int32_t i;
    shuffle_element_t e;

    begin_leaky_sec(rt);
    for (i = 0; i < bucket_len; ++i) {
      e.value = ob_read_next(bucket_ob);
      e.perm = ob_read_next(bucket_ob);
      if (e.perm != -1) {
        nob_write_at(nob, e.perm - begin_idx, e.value);
      }
    }
    end_leaky_sec(rt);

    free_cmo_runtime(rt);
  }

  free_shuffle_buckets(temp, temp_len);
}

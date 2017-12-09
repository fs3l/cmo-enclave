#include "./shuffle_bucket.h"
#include "algo.h"
#include "cmo.h"
#include "cmo_queue.h"
#include "utils.h"

static void _melbourne_shuffle_distribute(const int32_t* arr_in,
                                          const int32_t* perm_in, int32_t len,
                                          shuffle_bucket_p* buckets,
                                          int32_t num_of_bucket,
                                          int32_t p_log_len,
                                          int32_t bucket_idx_len)
{
  const int32_t write_output_len = 2 * num_of_bucket * p_log_len;
  int32_t* write_output = new int32_t[write_output_len];

  int32_t i, read_idx, write_idx, bucket_idx, write_output_idx;
  shuffle_element_t e;

  read_idx = 0;
  write_idx = 0;
  while (read_idx < len) {
    CMO_p rt = init_cmo_runtime();

    const int32_t read_len = min(len - read_idx, num_of_bucket);
    ReadObIterator_p arr_in_ob =
        init_read_ob_iterator(rt, arr_in + read_idx, read_len);
    ReadObIterator_p perm_in_ob =
        init_read_ob_iterator(rt, perm_in + read_idx, read_len);
    WriteObIterator_p write_ob =
        init_write_ob_iterator(rt, write_output, write_output_len);
    MultiQueue<shuffle_element_t> q(rt, num_of_bucket, num_of_bucket);

    begin_leaky_sec(rt);
    for (i = 0; i < num_of_bucket && read_idx < len; ++i) {
      e.value = ob_read_next(arr_in_ob);
      e.perm = ob_read_next(perm_in_ob);
      bucket_idx = e.perm / bucket_idx_len;
      bucket_idx = min(bucket_idx, num_of_bucket - 1);
      if (q.full()) cmo_abort(rt, "melbourne_shuffle: queue full");
      q.push_back(bucket_idx, e);
      ++read_idx;
    }

    for (bucket_idx = 0; bucket_idx < num_of_bucket; ++bucket_idx) {
      for (i = 0; i < p_log_len; ++i) {
        e.value = e.perm = -1;
        if (!q.empty(bucket_idx)) {
          q.front(bucket_idx, &e);
          q.pop_front(bucket_idx);
        }
        ob_write_next(write_ob, e.value);
        ob_write_next(write_ob, e.perm);
      }
    }
    end_leaky_sec(rt);

    write_output_idx = 0;
    for (bucket_idx = 0; bucket_idx < num_of_bucket; ++bucket_idx) {
      for (i = 0; i < p_log_len; ++i) {
        buckets[bucket_idx]->data[write_idx + 2 * i] =
            write_output[write_output_idx++];
        buckets[bucket_idx]->data[write_idx + 2 * i + 1] =
            write_output[write_output_idx++];
      }
    }

    write_idx += p_log_len * 2;
    free_cmo_runtime(rt);
  }

  delete[] write_output;
}

static void _melbourne_shuffle_cleanup(int32_t* arr_out,
                                       shuffle_bucket_p* buckets,
                                       int32_t num_of_bucket)
{
  for (int32_t bucket_idx = 0; bucket_idx < num_of_bucket; ++bucket_idx) {
    shuffle_bucket_p bucket = buckets[bucket_idx];
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
}

static void _melbourne_shuffle(const int32_t* arr_in, const int32_t* perm_in,
                               int32_t* arr_out, int32_t len,
                               int32_t blow_up_factor)
{
  const int32_t num_of_bucket =
      find_suitable_partitions(len, ceil(sqrt((double)len)));
  const int32_t log_len = ceil(log2((double)len));
  const int32_t p_log_len = blow_up_factor * log_len;
  const int32_t bucket_idx_len = len / num_of_bucket;
  const int32_t bucket_len = p_log_len * ceil((double)(len) / num_of_bucket);

  shuffle_bucket_p* buckets = init_empty_shuffle_buckets(
      num_of_bucket, bucket_len, 0, len, bucket_idx_len);

  _melbourne_shuffle_distribute(arr_in, perm_in, len, buckets, num_of_bucket,
                                p_log_len, bucket_idx_len);
  _melbourne_shuffle_cleanup(arr_out, buckets, num_of_bucket);

  free_shuffle_buckets(buckets, num_of_bucket);
}

void melbourne_shuffle(const int32_t* arr_in, const int32_t* perm_in,
                       int32_t* arr_out, int32_t len, int32_t blow_up_factor)
{
  int32_t* random = gen_random_sequence(len);
  int32_t* arr_random = new int32_t[len];
  int32_t* perm_random = new int32_t[len];

  _melbourne_shuffle(arr_in, random, arr_random, len, blow_up_factor);
  _melbourne_shuffle(perm_in, random, perm_random, len, blow_up_factor);
  _melbourne_shuffle(arr_random, perm_random, arr_out, len, blow_up_factor);

  delete[] random;
  delete[] arr_random;
  delete[] perm_random;
}

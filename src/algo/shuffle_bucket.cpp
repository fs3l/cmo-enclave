#include "./shuffle_bucket.h"

#include "utils.h"

shuffle_bucket_p init_empty_shuffle_bucket(int32_t len, int32_t begin_idx,
                                           int32_t end_idx)
{
  shuffle_bucket_p bucket = new shuffle_bucket_t;
  bucket->len = len;
  bucket->begin_idx = begin_idx;
  bucket->end_idx = end_idx;
  bucket->data = new int32_t[len * 2];
  for (int32_t i = 0; i < len; ++i) bucket->data[2 * i + 1] = -1;
  return bucket;
}

shuffle_bucket_p init_shuffle_bucket(const int32_t* arr, const int32_t* perm,
                                     int32_t len, int32_t begin_idx,
                                     int32_t end_idx)
{
  shuffle_bucket_p bucket = new shuffle_bucket_t;
  bucket->len = len;
  bucket->begin_idx = begin_idx;
  bucket->end_idx = end_idx;
  bucket->data = new int32_t[len * 2];
  for (int32_t i = 0; i < len; ++i) {
    bucket->data[2 * i] = arr[i];
    bucket->data[2 * i + 1] = perm[i];
  }
  return bucket;
}

void free_shuffle_bucket(shuffle_bucket_p bucket)
{
  delete[] bucket->data;
  delete bucket;
}

void randomize_shuffle_bucket(shuffle_bucket_p bucket)
{
  for (int32_t i = 0; i < bucket->len - 1; ++i) {
    int32_t j = random_int32() % bucket->len;
    swap(&bucket->data[2 * i], &bucket->data[2 * j]);
    swap(&bucket->data[2 * i + 1], &bucket->data[2 * j + 1]);
  }
}

void print_shuffle_bucket(const shuffle_bucket_p bucket, bool skip_invalid_perm)
{
  print_message("bucket[len=%d, begin_idx=%d, end_idx=%d]\n", bucket->len,
                bucket->begin_idx, bucket->end_idx);
  for (int32_t i = 0; i < bucket->len; ++i) {
    if (skip_invalid_perm && bucket->data[2 * i + 1] == -1) continue;
    print_message("<v=%d, p=%d>\n", bucket->data[2 * i],
                  bucket->data[2 * i + 1]);
  }
}

shuffle_bucket_p* init_empty_shuffle_buckets(int32_t num_of_bucket,
                                             int32_t bucket_len,
                                             int32_t begin_idx, int end_idx,
                                             int bucket_idx_len)
{
  shuffle_bucket_p* buckets = new shuffle_bucket_p[num_of_bucket];
  int32_t bucket_begin_idx = begin_idx;
  int32_t bucket_end_idx;
  for (int32_t i = 0; i < num_of_bucket; ++i) {
    if (i == num_of_bucket - 1)
      bucket_end_idx = end_idx;
    else
      bucket_end_idx = bucket_begin_idx + bucket_idx_len;
    buckets[i] =
        init_empty_shuffle_bucket(bucket_len, bucket_begin_idx, bucket_end_idx);
    bucket_begin_idx = bucket_end_idx;
  }
  return buckets;
}

void free_shuffle_buckets(shuffle_bucket_p* buckets, int num_of_bucket)
{
  for (int32_t i = 0; i < num_of_bucket; ++i) free_shuffle_bucket(buckets[i]);
  delete[] buckets;
}

ReadObIterator_p shuffle_bucket_init_read_ob(const shuffle_bucket_p bucket,
                                             CMO_p rt)
{
  return init_read_ob_iterator(rt, bucket->data, bucket->len * 2);
}

WriteObIterator_p shuffle_bucket_init_write_ob(shuffle_bucket_p bucket,
                                               CMO_p rt)
{
  return init_write_ob_iterator(rt, bucket->data, bucket->len * 2);
}

ReadObIterator_p shuffle_bucket_init_read_ob(const shuffle_bucket_p bucket,
                                             CMO_p rt, int32_t start_idx,
                                             int32_t len)
{
  return init_read_ob_iterator(rt, bucket->data + start_idx * 2, len * 2);
}

WriteObIterator_p shuffle_bucket_init_write_ob(shuffle_bucket_p bucket,
                                               CMO_p rt, int32_t start_idx,
                                               int32_t len)
{
  return init_write_ob_iterator(rt, bucket->data + start_idx * 2, len * 2);
}

int32_t find_suitable_partitions(int32_t len, int32_t partition)
{
  return min(partition,
             (int32_t)ceil((double)len / ceil((double)len / partition)));
}

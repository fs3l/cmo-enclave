#include "./shuffle_bucket.h"

#include <stdio.h>
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
  // TODO: replace printf with ecall.
  printf("bucket[len=%d, begin_idx=%d, end_idx=%d]\n", bucket->len,
         bucket->begin_idx, bucket->end_idx);
  for (int32_t i = 0; i < bucket->len; ++i) {
    if (skip_invalid_perm && bucket->data[2 * i + 1] == -1) continue;
    printf("<v=%d, p=%d>\n", bucket->data[2 * i], bucket->data[2 * i + 1]);
  }
}

int32_t find_suitable_paritions(int32_t len, int32_t partition)
{
  return min(partition,
             (int32_t)ceil((double)len / ceil((double)len / partition)));
}

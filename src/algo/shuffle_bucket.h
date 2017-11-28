#ifndef SHUFFLE_BUCKET_H
#define SHUFFLE_BUCKET_H

#include <stdint.h>

#include "cmo.h"

struct shuffle_element {
  int32_t value;
  int32_t perm;
};
typedef struct shuffle_element shuffle_element_t;

struct shuffle_bucket {
  int32_t len;
  int32_t begin_idx;
  int32_t end_idx;
  int32_t* data;  // layout: [value, perm], ....
};
typedef struct shuffle_bucket shuffle_bucket_t;
typedef struct shuffle_bucket* shuffle_bucket_p;

shuffle_bucket_p init_empty_shuffle_bucket(int32_t len, int32_t begin_idx,
                                           int32_t end_idx);
shuffle_bucket_p init_shuffle_bucket(const int32_t* arr, const int32_t* perm,
                                     int32_t len, int32_t begin_idx,
                                     int32_t end_idx);
void free_shuffle_bucket(shuffle_bucket_p bucket);
void randomize_shuffle_bucket(shuffle_bucket_p bucket);
void print_shuffle_bucket(const shuffle_bucket_p bucket,
                          bool skip_invalid_perm = false);

shuffle_bucket_p* init_empty_shuffle_buckets(int32_t num_of_bucket,
                                             int32_t bucket_len,
                                             int32_t begin_idx, int end_idx,
                                             int bucket_idx_len);
void free_shuffle_buckets(shuffle_bucket_p* buckets, int num_of_bucket);

ReadObIterator_p shuffle_bucket_init_read_ob(const shuffle_bucket_p bucket,
                                             CMO_p rt);
WriteObIterator_p shuffle_bucket_init_write_ob(shuffle_bucket_p bucket,
                                               CMO_p rt);

// find a partition number p such that the len can be divided
// to p partitions with each partition has the size len / p.
int32_t find_suitable_paritions(int32_t len, int32_t parition);

#endif

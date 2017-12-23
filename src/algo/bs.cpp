#include "./shuffle_bucket.h"
#include "algo.h"
#include "cmo.h"
#include "cmo_queue.h"
#include "utils.h"
#define SCAN 0
#include <stdio.h>
static int search(int32_t* index, int32_t index_len, int32_t key) {
  int32_t len = index_len;
  int32_t low = 0;
  int32_t high = len-1;
  int32_t mid = 0;
  int res = 0;
  while(low<=high) {
    mid = (high-low)/2 + low;
    res = index[mid];
    if (res==key) break;
    if (res < key) low = mid + 1;
    else high = mid -1;
  }
  return mid;
}

static int search(ReadNobArray_p idx, int32_t key) {
  int32_t len = idx->len;
  int32_t low = 0;
  int32_t high = len-1;
  int32_t mid = 0;
  int res = 0;
  while(low<=high) {
    mid = (high-low)/2 + low;
    res = nob_read_at(idx,mid);
    if (res==key) break;
    if (res < key) low = mid + 1;
    else high = mid -1;
  }
  return mid;
}

#if SCAN
static void _binary_search(int32_t* index, int32_t index_len, int32_t work_len)
{
  int res = 0;
  for(int i=0;i<10;i++) {
    for(int i=0;i<index_len;i++) {
      bool cond  = (index[i] == 1);
      cmove_int32(cond,&i,&res);
    }
    for(int i=0;i<index_len;i++) {
      bool cond  = (index[i] == 512*1024);
      cmove_int32(cond,&i,&res);
    }
  }
}
#else
static void _binary_search(int32_t* index, int32_t index_len, int32_t work_len)
{
  CMO_p rt = init_cmo_runtime();
  ReadNobArray_p nob =
    init_read_nob_array(rt, index , index_len);
  for(int j=0;j<10;j++) {
    begin_leaky_sec(rt);
    for(int i=0;i<1000;i++) {
      search(nob,1);
      search(nob,512*1024);
    }
    end_leaky_sec(rt);
  }
  free_cmo_runtime(rt);
}
#endif

void binary_search(int32_t* index, int32_t index_len, int32_t work_len)
{
  _binary_search(index,index_len,work_len);

}

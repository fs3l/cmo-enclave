#include "./shuffle_bucket.h"
#include "algo.h"
#include "cmo.h"
#include "cmo_queue.h"
#include "utils.h"
#define SCAN 1
#include <stdio.h>
static void _wordcnt(int32_t* input, int32_t* output, int32_t len)
{
  CMO_p rt = init_cmo_runtime();
  NobArray_p nob =
    init_nob_array(rt, output , 26);
  ReadObIterator_p  ob = init_read_ob_iterator(rt, input,len);
  begin_leaky_sec(rt);
  int32_t addr = 0;
  int32_t v = 0;
  for(int i=0;i<len;i++) {
    addr = ob_read_next(ob);
    v = nob_read_at(nob,addr);
    v++;
    nob_write_at(nob,addr,v);
  } 
  end_leaky_sec(rt);
  free_cmo_runtime(rt);
}

void wordcnt(int32_t* input, int32_t* output,int32_t len)
{
  _wordcnt(input,output,len);
}

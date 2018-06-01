#include "algo.h"
#include "cmo.h"

void trace_back(const int32_t* input,  int32_t* arr_out, int32_t len, int32_t col,int32_t m, int32_t n)
{
  CMO_p rt = init_cmo_runtime();
  ReadObIterator_p d = init_read_ob_iterator(rt, input, len);
  NobArray_p o = init_nob_array(rt, arr_out, len);

  begin_leaky_sec(rt);

  end_leaky_sec(rt);
    while(m>0|n>0){
      if(input[(m-1)*col+(n-1)] <= input[m*col+(n-1)]&& input[(m-1)*col+(n-1)] <= input[(m-1)*col+n]){
          m--;
          n--;
      }
      else if(input[m*col+n-1] <= input[(m-1)*col+n-1]&&input[m*col+n-1] <= input[(m-1)*col+n]){
          n--;
      }
      else if(input[(m-1)*col+n] <= input[(m-1)*col+n-1]&&input[(m-1)*col+n] <= input[m*col+n-1]){
          m--;
      }
	nob_write_at(o,m*col+n,input[m*col+n]);
  }

  free_cmo_runtime(rt);
}

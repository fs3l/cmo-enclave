#include "algo.h"
#include <vector>
#include <map>
void map_wc(int32_t key1, std::vector<int32_t> value, void* data){
  kvpair_p kvp=new kvpair_t;
  kvp->key=value[0];
  kvp->value.push_back(1);
  emit_interm(*kvp);
}

void reduce_wc(int32_t key2, std::vector<std::vector<int>> values,std::map<int,std::vector<int>> &output){
  int32_t result = 0;
  for(std::vector<int> a:values){
    result += a[0];
  }
  kvpair_p kvp=new kvpair_t;
  kvp->key=key2;
  kvp->value.push_back(result);
  emit(kvp,output);
}

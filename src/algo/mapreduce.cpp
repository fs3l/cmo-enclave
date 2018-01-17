#include "./shuffle_bucket.h"
#include "algo.h"
#include "cmo.h"
#include "cmo_queue.h"
#include "cmo_array.h"
#include "utils.h"
#include <stdio.h>
#include <map>
#include <vector>
//mapreduce rt
std::vector<kvpair_t> interm; 
std::vector<kvpair_t> interm_shuffled; 
std::vector<kvpair_t> reducer0_in; 
std::vector<kvpair_t> reducer1_in; 
void emit_interm(kvpair_t kv){
  interm.push_back(kv);
}

void emit(kvpair_p kvp,std::map<int,std::vector<int>> &output){
  output.emplace(kvp->key,kvp->value);
}

void mapper(std::vector<kvpair_t> input_pt, int32_t start, int32_t pt_size, void (*map)(int32_t,std::vector<int>)){
  for(int32_t i=0; i<pt_size; i++){
    map(input_pt[start+i].key,input_pt[start+i].value);
  }
}

void reducer(std::vector<kvpair_t> input, void (*reduce)(int32_t,std::vector<std::vector<int>>,std::map<int,std::vector<int>> &output),std::map<int,std::vector<int>> &output){
  std::map<int,std::vector<std::vector<int>>> table;
  for(kvpair_t kv:input){
    if(table.find(kv.key) != table.end()) {
      std::vector<std::vector<int>> &v  = table.at(kv.key);
      v.push_back(kv.value);
    } else {
      std::vector<std::vector<int>> v;
      v.push_back(kv.value);
      table.emplace(kv.key,v);
    }
  }
  
  for (std::map<int,std::vector<std::vector<int>>>::iterator it = table.begin();it!=table.end();it++) {
    reduce(it->first,it->second,output);
  }
}

/**
  two mappers, two reducers
 */
void mapreduce_rt(std::vector<kvpair_t> input_sorted,  int n, void (*map)(int32_t,std::vector<int>), void (*reduce)(int32_t,std::vector<std::vector<int>>,std::map<int,std::vector<int>>&), std::map<int,std::vector<int>> &output){
  mapper(input_sorted, 0,n/2, map);
  mapper(input_sorted, n/2, n/2 + n%2, map);

  int32_t interm_size = interm.size();
  int32_t* keys_to_shuffle = new int32_t[interm_size];
  int32_t* vals_to_shuffle = new int32_t[interm_size];
  int32_t* keys_to_shuffled = new int32_t[interm_size];
  int32_t* vals_to_shuffled = new int32_t[interm_size];
  int32_t* perm = gen_random_sequence(interm_size);
  for(int i=0;i<interm_size;i++) keys_to_shuffle[i] = interm[i].key;
  for(int i=0;i<interm_size;i++) vals_to_shuffle[i] = interm[i].value[0];
  melbourne_shuffle(keys_to_shuffle,perm,keys_to_shuffled,interm_size,1);
  melbourne_shuffle(vals_to_shuffle,perm,vals_to_shuffled,interm_size,1);
  for(int i=0;i<interm_size;i++) {
    kvpair_p kv = new kvpair_t;
    kv->key = keys_to_shuffled[i];
    kv->value.push_back(vals_to_shuffled[i]);
    interm_shuffled.push_back(*kv);
  }
  //mr-shuffle
  for(kvpair_t kv : interm_shuffled){
    kvpair_p it1 = new kvpair_t;
    it1->key = kv.key;
    it1->value = kv.value;
    if(kv.key%2== 0){
      reducer0_in.push_back(*it1); 
    } else {
      reducer1_in.push_back(*it1); 
    }
  }
  reducer(reducer0_in,reduce,output);
  reducer(reducer1_in,reduce,output);
}

void map_wc(int32_t key1, std::vector<int32_t> value){
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

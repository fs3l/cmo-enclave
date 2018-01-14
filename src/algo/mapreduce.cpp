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
kvpair_p interm_first=0,interm_last=0;

kvpair_p reducer0_in_first=0,reducer0_in_last=0;
kvpair_p reducer1_in_first=0,reducer1_in_last=0;

void emit_interm(kvpair_p kvp){
  if(interm_first == 0){
    interm_first = kvp;
    interm_last = kvp;
  } else {
    interm_last->next = kvp;
    interm_last = kvp;
  }
}

void emit(kvpair_p kvp){
  printf("output emit: <%d,%d>\n", kvp->key,kvp->value);
}

void mapper(kvpair_p input_pt, int32_t pt_size, void (*map)(int32_t,int32_t)){
  for(int32_t i=0; i<pt_size; i++){
    map(input_pt[i].key,input_pt[i].value);
  }
}

void reducer(kvpair_p start, kvpair_p end, void (*reduce)(int32_t,std::vector<int>)){
  std::map<int,std::vector<int>> table;
  for(kvpair_p it=start; it!=end; it=it->next){
    if(table.find(it->key) != table.end()) {
      std::vector<int> &v  = table.at(it->key);
      v.push_back(it->value);
    } else {
      std::vector<int> v;
      v.push_back(it->value);
      table.emplace(it->key,v);
    }
  }
  if(table.find(end->key) != table.end()) {
    std::vector<int> &v  = table.at(end->key);
    v.push_back(end->value);
  } else {
    std::vector<int> v;
    v.push_back(end->value);
    table.emplace(end->key,v);
  }

  for (std::map<int,std::vector<int>>::iterator it = table.begin();it!=table.end();it++) {
    std::vector<int> &v = it->second;
    reduce(it->first,it->second);
  }
}

/**
  two mappers, two reducers
 */
void mapreduce_rt(kvpair_p input_sorted, int32_t n, void (*map)(int32_t,int32_t), void (*reduce)(int32_t,std::vector<int>)){
  mapper(input_sorted, n/2, map);
  mapper(input_sorted + n/2, n/2 + n%2, map);
  for(kvpair_p it = interm_first; it != interm_last; it = it->next){
    it->r = it->key % 2; //TODO
  }
  interm_last->r = interm_last->key % 2;

  //mr-shuffle
  for(kvpair_p it = interm_first; it != interm_last; it = it->next){
    kvpair_p it1 = new kvpair_t;
    it1->key = it->key;
    it1->value = it->value;
    if(it->r == 0){
      if(reducer0_in_first == 0){
        reducer0_in_first = it1;
        reducer0_in_last = it1;
      } else {
        reducer0_in_last->next = it1;
        reducer0_in_last = it1;
      }
    } else {
      if(reducer1_in_first == 0){
        reducer1_in_first = it1;
        reducer1_in_last = it1;
      } else {
        reducer1_in_last->next = it1;
        reducer1_in_last = it1;
      }
    }
  }
  kvpair_p it1 = new kvpair_t;
  it1->key = interm_last->key;
  it1->value = interm_last->value;
  if(interm_last->r == 0){
    if(reducer0_in_first == 0){
      reducer0_in_first = it1;
      reducer0_in_last = it1;
    } else {
      reducer0_in_last->next = it1;
      reducer0_in_last = it1;
    }
  } else {
    if(reducer1_in_first == 0){
      reducer1_in_first = it1;
      reducer1_in_last = it1;
    } else {
      reducer1_in_last->next = it1;
      reducer1_in_last = it1;
    }
  }
  
  reducer(reducer0_in_first, reducer0_in_last,reduce);
  reducer(reducer1_in_first, reducer1_in_last,reduce);
}

//mapreduce udf

void map_wc(int32_t key1, int32_t value1){
  kvpair_p kvp1=new kvpair_t;
  kvp1->key=value1%4;
  kvp1->value=1;
  emit_interm(kvp1);

  value1>>=2;
  kvpair_p kvp2=new kvpair_t;
  kvp2->key=value1%4;
  kvp2->value=1;
  emit_interm(kvp2);

  value1>>=2;
  kvpair_p kvp3=new kvpair_t;
  kvp3->key=value1%4;
  kvp3->value=1;
  emit_interm(kvp3);
}

void reduce_wc(int32_t key2, std::vector<int> values){
  int32_t result = 0;
  for(int32_t a:values){
    result += a;
  }
  kvpair_p kvp=new kvpair_t;
  kvp->key=key2;
  kvp->value=result;
  emit(kvp);
}

///TODO

/**
  static int64_t _kmeans_distance(int32_t x1, int32_t y1, int32_t x2, int32_t y2)
  {
  int64_t delta_x = x1 - x2;
  int64_t delta_y = y1 - y2;
  return delta_x * delta_x + delta_y * delta_y;
  }

  static void _kmeans_map(const int32_t* x_in, const int32_t* y_in, int32_t len,
  const int32_t* center_x, const int32_t* center_y,
  int32_t k, int32_t* result)
  {
  for (int32_t i = 0; i < len; ++i) {
  int32_t id = 0;
  int64_t dist =
  _kmeans_distance(x_in[i], y_in[i], center_x[id], center_y[id]);
  for (int32_t j = 1; j < k; ++j) {
  int64_t new_dist =
  _kmeans_distance(x_in[i], y_in[i], center_x[j], center_y[j]);
  bool cond = new_dist < dist;
  cmove_int64(cond, &new_dist, &dist);
  cmove_int32(cond, &j, &id);
  }
  result[i] = id;
  }
  }

  struct kmeans_center {
  int64_t x;
  int64_t y;
  int32_t count;
  };
  typedef struct kmeans_center kmeans_center_t;

  static int32_t _kmeans_sort_centers_partition(Array<kmeans_center_t>& centers,
  int32_t low, int32_t high)
  {
  kmeans_center_t pivot, e1, e2;
  centers.read(high, &pivot);
  int32_t i = low - 1;
  for (int32_t j = low; j < high; ++j) {
  centers.read(j, &e1);
  if (e1.x < pivot.x || (e1.x == pivot.x && e1.y < pivot.y)) {
  i++;
  centers.read(i, &e2);
  centers.write(i, &e1);
  centers.write(j, &e2);
  }
  }
  centers.read(i + 1, &e1);
  centers.read(high, &e2);
  centers.write(high, &e1);
  centers.write(i + 1, &e2);
  return i + 1;
  }

  static void _kmeans_sort_centers(Array<kmeans_center_t>& centers, int32_t low,
  int32_t high)
  {
  if (low < high) {
  int32_t p = _kmeans_sort_centers_partition(centers, low, high);
  _kmeans_sort_centers(centers, low, p - 1);
  _kmeans_sort_centers(centers, p + 1, high);
  }
  }

  static void _kmeans_reduce(const int32_t* x_in, const int32_t* y_in,
  const int32_t* ids, int32_t len, int32_t k,
  int32_t* center_x, int32_t* center_y)
  {
  CMO_p rt = init_cmo_runtime();
Array<kmeans_center_t> centers(rt, k);
ReadObIterator_p x_ob = init_read_ob_iterator(rt, x_in, len);
ReadObIterator_p y_ob = init_read_ob_iterator(rt, x_in, len);
ReadObIterator_p ids_ob = init_read_ob_iterator(rt, ids, len);

int32_t i, id;
kmeans_center_t center;
center.x = 0;
center.y = 0;
center.count = 0;

for (i = 0; i < k; ++i) centers.write_leaky(i, &center);

begin_leaky_sec(rt);

for (i = 0; i < len; ++i) {
  id = ob_read_next(ids_ob);
  centers.read(id, &center);
  center.x += ob_read_next(x_ob);
  center.y += ob_read_next(y_ob);
  center.count++;
  centers.write(id, &center);
}

for (i = 0; i < k; ++i) {
  centers.read(i, &center);
  center.x /= center.count;
  center.y /= center.count;
  centers.write(i, &center);
}

_kmeans_sort_centers(centers, 0, k - 1);

end_leaky_sec(rt);

for (i = 0; i < k; ++i) {
  centers.read_leaky(i, &center);
  center_x[i] = center.x;
  center_y[i] = center.y;
}

free_cmo_runtime(rt);
}

static bool _kmeans_stop(int32_t k, const int32_t* center_x,
    const int32_t* center_y, const int32_t* new_center_x,
    const int32_t* new_center_y)
{
  int32_t result = 0;
  int32_t changed = 1;
  for (int32_t i = 0; i < k; ++i) {
    cmove_int32(center_x[i] != new_center_x[i], &changed, &result);
    cmove_int32(center_y[i] != new_center_y[i], &changed, &result);
  }

  return result != changed;
}

void kmeans(const int32_t* x_in, const int32_t* y_in, int32_t len, int32_t k,
    int32_t* result)
{
  int32_t* center_x = new int32_t[k];
  int32_t* center_y = new int32_t[k];

  for (int32_t i = 0; i < k; ++i) {
    center_x[i] = x_in[i];
    center_y[i] = y_in[i];
  }

  int32_t iteration = 0;
  while (iteration++ < 100000) {
    _kmeans_map(x_in, y_in, len, center_x, center_y, k, result);
    int32_t* new_center_x = new int32_t[k];
    int32_t* new_center_y = new int32_t[k];
    _kmeans_reduce(x_in, y_in, result, len, k, new_center_x, new_center_y);

    if (_kmeans_stop(k, center_x, center_y, new_center_x, new_center_y)) {
      delete[] new_center_x;
      delete[] new_center_y;
      break;
    } else {
      delete[] center_x;
      delete[] center_y;
      center_x = new_center_x;
      center_y = new_center_y;
    }
  }

  delete[] center_x;
  delete[] center_y;
}
*/

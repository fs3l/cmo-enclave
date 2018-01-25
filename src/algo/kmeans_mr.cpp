#include "algo.h"
#include "utils.h"
#include <vector>
#include <map>

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


static int32_t _kmeans_sort_centers_partition(int* center_x, int* center_y,
    int32_t low, int32_t high)
{
  int pivot_x;
  int pivot_y;
  int e1_x;
  int e1_y;
  int e2_x;
  int e2_y;
  pivot_x = center_x[high];
  pivot_y = center_y[high];
  int32_t i = low - 1;
  for (int32_t j = low; j < high; ++j) {
    e1_x = center_x[j];
    e1_y = center_y[j];
    if (e1_x < pivot_x || (e1_x == pivot_x && e1_y < pivot_y)) {
      i++;
      e2_x = center_x[i];
      e2_y = center_y[i];
      center_x[i] = e1_x;
      center_y[i] = e1_y;
      center_x[j] = e2_x;
      center_y[j] = e2_y;
    }
  }
  //centers.read(i + 1, &e1);
  e1_x = center_x[i+1];
  e1_y = center_y[i+1];
  //centers.read(high, &e2);
  e2_x = center_x[high];
  e2_y = center_y[high];
  //centers.write(high, &e1);
  center_x[high] = e1_x;
  center_y[high] = e1_y;
  //centers.write(i + 1, &e2);
  center_x[i+1] = e2_x;
  center_y[i+1] = e2_y;
  return i + 1;
}

static void _kmeans_sort_centers(int* center_x, int* center_y, int32_t low,
    int32_t high)
{
  if (low < high) {
    int32_t p = _kmeans_sort_centers_partition(center_x, center_y,low, high);
    _kmeans_sort_centers(center_x,center_y, low, p - 1);
    _kmeans_sort_centers(center_x,center_y, p + 1, high);
  }
}



static int64_t _kmeans_distance(int32_t x1, int32_t y1, int32_t x2, int32_t y2)
{
  int64_t delta_x = x1 - x2;
  int64_t delta_y = y1 - y2;
  return delta_x * delta_x + delta_y * delta_y;
}

void map_kmeans(int32_t key1, std::vector<int32_t> value, void* aux_data){
  int32_t id = 0;
  kmeans_aux_p aux =  (kmeans_aux_p)aux_data;
  int* center_x = aux->center_x;
  int* center_y = aux->center_y;
  int k = aux->k;
  int64_t dist =
    _kmeans_distance(value[0], value[1], center_x[id], center_y[id]);
  for (int32_t j = 1; j < k; ++j) {
    int64_t new_dist =
      _kmeans_distance(value[0], value[1], center_x[j], center_y[j]);
    bool cond = new_dist < dist;
    cmove_int64(cond, &new_dist, &dist);
    cmove_int32(cond, &j, &id);
  }
  kvpair_p kvp=new kvpair_t;
  kvp->key=id;
  kvp->value.push_back(value[0]);
  kvp->value.push_back(value[1]);
  aux->meta_output[key1] = id;
  emit_interm(*kvp);
}

void reduce_kmeans(int32_t key2, std::vector<std::vector<int>> values,std::map<int,std::vector<int>> &output){
  int64_t center_x = 0;
  int64_t center_y = 0;
  int32_t result_x = 0;
  int32_t result_y = 0;
  for(std::vector<int> a:values){
    center_x += a[0];
    center_y += a[1];
  }
  result_x = center_x/values.size();
  result_y = center_y/values.size();
  kvpair_p kvp=new kvpair_t;
  kvp->key=key2;
  kvp->value.push_back(result_x);
  kvp->value.push_back(result_y);
  emit(kvp,output);
}

/*
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
 */
void kmeans_mr(const int32_t* x_in, const int32_t* y_in, int32_t len, int32_t k,
    int32_t* result)
{
  int32_t* center_x = new int32_t[k];
  int32_t* center_y = new int32_t[k];
  int32_t* meta_output = result;

  for (int32_t i = 0; i < k; ++i) {
    center_x[i] = x_in[i];
    center_y[i] = y_in[i];
  }

  kmeans_aux_p aux = new kmeans_aux_t;
  aux->center_x = center_x;
  aux->center_y = center_y;
  aux->k = k;
  aux->meta_output = result;
  std::vector<kvpair_t> input_sorted(len);
  std::map<int,std::vector<int>> output;
  for(int i=0;i<len;i++) {
    input_sorted[i].value.push_back(x_in[i]);
    input_sorted[i].value.push_back(y_in[i]);
    input_sorted[i].key = i;
  }
  int32_t iteration = 0;
  while (iteration++ < 100) {
    mapreduce_rt(input_sorted, len, map_kmeans, reduce_kmeans,output,aux);

    int32_t* new_center_x = new int32_t[k];
    int32_t* new_center_y = new int32_t[k];
    int p =0;
    for (std::map<int,std::vector<int>>::iterator it = output.begin();it!=output.end();it++) {
      new_center_x[p] = it->second[0];
      new_center_y[p] = it->second[1];
      p++;
    }
    
    _kmeans_sort_centers(new_center_x, new_center_y,0, k - 1);
    for(int i=0;i<k;i++) {
        //printf("new_center_x[%d]=%d,new_center_y[%d]=%d\n",i,new_center_x[i],i,new_center_y[i]);
    }
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
  printf("iteration=%d\n",iteration);
  delete[] center_x;
  delete[] center_y;
}

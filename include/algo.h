#ifndef ALGO_H
#define ALGO_H

#include <stdint.h>
#include <vector>
#include <map>
#ifdef __cplusplus
extern "C" {
#endif

void naive_shuffle(const int32_t* arr_in, const int32_t* perm_in,
                   int32_t* arr_out, int32_t len);

void melbourne_shuffle(const int32_t* arr_in, const int32_t* perm_in,
                       int32_t* arr_out, int32_t len, int32_t blow_up_factor);

void cache_shuffle(const int32_t* arr_in, const int32_t* perm_in,
                   int32_t* arr_out, int32_t len, double epsilon,
                   int32_t mem_cap);

void merge_sort(const int32_t* arr_in, int32_t* arr_out, int32_t len,
                int32_t blow_up_factor);

int64_t merger(int32_t len, int32_t start, int32_t* input, int32_t* values);
void binary_search(int32_t* index, int32_t index_len, int32_t work_len);
void wordcnt(int32_t* input, int32_t* output, int32_t len);
struct count_result {
  int32_t element;
  int32_t count;
};
typedef struct count_result count_result_t;

void element_count(const int32_t* arr_in, int32_t len, int32_t uniq_elememts,
                   count_result_t* result);

void kmeans(const int32_t* x_in, const int32_t* y_in, int32_t len, int32_t k,
            int32_t* result);
void kmeans_mr(const int32_t* x_in, const int32_t* y_in, int32_t len, int32_t k,
            int32_t* result);

struct kvpair {
  int32_t key;
  std::vector<int32_t> value;
  kvpair* next;
  int32_t r;
};
typedef struct kvpair  kvpair_t;
typedef struct kvpair* kvpair_p;

struct kmeans_aux {
  int32_t k;
  int32_t* center_x;
  int32_t* center_y;
  int32_t* meta_output;
};

struct graph_ve {
  int32_t src_v;
  int32_t dst_v;
  int32_t is_v;
  int32_t data;
};
typedef struct graph_ve graph_ve_t;
typedef struct graph_ve* graph_ve_p;
typedef struct kmeans_aux kmeans_aux_t;
typedef struct kmeans_aux* kmeans_aux_p;

void mapreduce_rt(std::vector<kvpair_t> input_sorted, int32_t n, void (*map)(int32_t,std::vector<int>, void*), void (*reduce)(int32_t,std::vector<std::vector<int>>,std::map<int,std::vector<int>>&),std::map<int,std::vector<int>> &output, void* aux);
void map_wc(int32_t key1, std::vector<int> value1, void* aux);
void reduce_wc(int32_t key2, std::vector<std::vector<int>> values, std::map<int,std::vector<int>> &output);
void emit_interm(kvpair_t kv);
void emit(kvpair_p kvp,std::map<int,std::vector<int>> &output);
void graphsc(std::vector<graph_ve_t> &graph);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

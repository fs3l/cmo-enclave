#include "./shuffle_bucket.h"
#include "algo.h"
#include "cmo.h"
#include "cmo_queue.h"
#include "utils.h"
#define SCAN 1
#include <stdio.h>
#include <algorithm>
int func_scatter(int val) {
  return val;
}
bool comp_scatter(graph_ve_t& e1, graph_ve_t& e2){
  if (e1.src_v < e2.src_v) return true;
  if (e1.src_v == e2.src_v) return (e1.is_v>e2.is_v);
  return false;
}
bool comp_gather(graph_ve_t& e1, graph_ve_t& e2){
  if (e1.dst_v < e2.dst_v) return true;
  if (e1.dst_v == e2.dst_v) return (e1.is_v<e2.is_v);
  return false;
}

static void _scatter(std::vector<graph_ve_t>& graph) {
  int val = 0;
  for(int i=0;i<graph.size();i++){
    graph_ve_t& ve = graph[i];
    if (ve.is_v) val = ve.data;
    else ve.data = func_scatter(val); 
  }
}
static void _gather(std::vector<graph_ve_t>& graph) {
  int agg = 0;
  for(int i=0;i<graph.size();i++){
    graph_ve_t& ve = graph[i];
    if (ve.is_v) {ve.data=agg;agg=0;}
    else agg=agg+ve.data; 
  }
}

static void shuffle(std::vector<graph_ve_t>& graph) {
int32_t interm_size = graph.size();
  int32_t* field1_to_shuffle = new int32_t[interm_size];
  int32_t* field2_to_shuffle = new int32_t[interm_size];
  int32_t* field3_to_shuffle = new int32_t[interm_size];
  int32_t* field4_to_shuffle = new int32_t[interm_size];
  int32_t* field1_to_shuffled = new int32_t[interm_size];
  int32_t* field2_to_shuffled = new int32_t[interm_size];
  int32_t* field3_to_shuffled = new int32_t[interm_size];
  int32_t* field4_to_shuffled = new int32_t[interm_size];
  int32_t* perm = gen_random_sequence(interm_size);
  for(int i=0;i<interm_size;i++) field1_to_shuffle[i] = graph[i].src_v;
  for(int i=0;i<interm_size;i++) field2_to_shuffle[i] = graph[i].dst_v;
  for(int i=0;i<interm_size;i++) field3_to_shuffle[i] = graph[i].is_v;
  for(int i=0;i<interm_size;i++) field4_to_shuffle[i] = graph[i].data;
  melbourne_shuffle(field1_to_shuffle,perm,field1_to_shuffled,interm_size,1);
  melbourne_shuffle(field2_to_shuffle,perm,field2_to_shuffled,interm_size,1);
  melbourne_shuffle(field3_to_shuffle,perm,field3_to_shuffled,interm_size,1);
  melbourne_shuffle(field4_to_shuffle,perm,field4_to_shuffled,interm_size,1);
  graph.clear();
  for(int i=0;i<interm_size;i++) {
    graph_ve_p ve = new graph_ve_t;
    ve->src_v = field1_to_shuffled[i];
    ve->dst_v = field2_to_shuffled[i];
    ve->is_v = field3_to_shuffled[i];
    ve->data = field4_to_shuffled[i];
    graph.push_back(*ve);
  }

}

static void scatter_shuffle_sort(std::vector<graph_ve_t>& graph) {
  shuffle(graph);  
  std::sort(graph.begin(),graph.end(),comp_scatter);
}

static void gather_shuffle_sort(std::vector<graph_ve_t>& graph) {
  shuffle(graph);  
  std::sort(graph.begin(),graph.end(),comp_gather);
}

static void _graphsc(std::vector<graph_ve_t>& graph)
{
  scatter_shuffle_sort(graph);
  _scatter(graph);
  gather_shuffle_sort(graph);
  _gather(graph);
}

void graphsc(std::vector<graph_ve_t>& graph)
{
  _graphsc(graph);
}

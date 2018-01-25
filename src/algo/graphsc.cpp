#include "./shuffle_bucket.h"
#include "algo.h"
#include "cmo.h"
#include "cmo_queue.h"
#include "utils.h"
#define SCAN 1
#include <stdio.h>
#include <algorithm>
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
    if (ve.is_v && ve.o_links!=0) val = ve.data/ve.o_links;
    else ve.data = val; 
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
static void _graphsc(std::vector<graph_ve_t>& graph)
{
  std::sort(graph.begin(),graph.end(),comp_scatter);
  _scatter(graph);
  std::sort(graph.begin(),graph.end(),comp_gather);
  _gather(graph);
  for(int i=0;i<graph.size();i++){
    graph_ve_t& ve = graph[i];
    printf("ve.is_v=%d,ve.src_v=%d,ve.dst_v=%d,ve.data=%d\n",ve.is_v,ve.src_v,ve.dst_v,ve.data);
  }
}

void graphsc(std::vector<graph_ve_t>& graph)
{
  _graphsc(graph);
}

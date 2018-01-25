#include "./helper.h"

#ifdef SGX_APP
#include "./sgx_helper.h"

#include "algo_u.h"
#else
#include "algo.h"
#endif
#include <sys/time.h>
BOOST_AUTO_TEST_CASE(graphsc_test)
{
  std::vector<graph_ve_t> graph;
  struct timeval begin,end;
  graph_ve_p v = new graph_ve_t;
  v->src_v = 1;
  v->dst_v = 1;
  v->is_v  = 1;
  v->data = 30;
  v->o_links = 3;
  graph.push_back(*v);
  v = new graph_ve_t;
  v->src_v = 2;
  v->dst_v = 2;
  v->is_v  = 1;
  v->data = 30;
  v->o_links = 1;
  graph.push_back(*v);
  v = new graph_ve_t;
  v->src_v = 3;
  v->dst_v = 3;
  v->is_v  = 1;
  v->data = 30;
  graph.push_back(*v);
  v = new graph_ve_t;
  v->src_v = 4;
  v->dst_v = 4;
  v->is_v  = 1;
  v->data = 30;
  v->o_links = 1;
  graph.push_back(*v);
  graph_ve_p e = new graph_ve_t;
  e->src_v = 1;
  e->dst_v = 2;
  e->is_v  = 0;
  graph.push_back(*e);
  e = new graph_ve_t;
  e->src_v = 1;
  e->dst_v = 3;
  e->is_v  = 0;
  graph.push_back(*e);
  e = new graph_ve_t;
  e->src_v = 1;
  e->dst_v = 4;
  e->is_v  = 0;
  graph.push_back(*e);
  e = new graph_ve_t;
  e->src_v = 2;
  e->dst_v = 3;
  e->is_v  = 0;
  graph.push_back(*e);
  e = new graph_ve_t;
  e->src_v = 4;
  e->dst_v = 3;
  e->is_v  = 0;
  graph.push_back(*e);

  gettimeofday(&begin,NULL);
  graphsc(graph);
  gettimeofday(&end,NULL);
  printf("time spent=%ld\n",1000000*(end.tv_sec-begin.tv_sec)+end.tv_usec-begin.tv_usec);
  for(int i=0;i<graph.size();i++){
    graph_ve_t& ve = graph[i];
    if(ve.is_v==1&&ve.src_v==1) BOOST_CHECK(ve.data == 0);
    if(ve.is_v==1&&ve.src_v==2) BOOST_CHECK(ve.data == 10);
    if(ve.is_v==1&&ve.src_v==3) BOOST_CHECK(ve.data == 70);
    if(ve.is_v==1&&ve.src_v==4) BOOST_CHECK(ve.data == 10);
  }
}

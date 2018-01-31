#include "./helper.h"

#ifdef SGX_APP
#include "./sgx_helper.h"

#include "algo_u.h"
#else
#include "algo.h"
#endif
#include <sys/time.h>

#define N_V 400
#define N_E 600
BOOST_AUTO_TEST_CASE(pagerank_test)
{
  std::vector<graph_ve_t> graph;
  struct timeval begin,end;
  graph_ve_p v;
  graph_ve_p e;
  for(int i=0;i<N_V;i++) {
    v = new graph_ve_t;
    v->src_v = i+1;
    v->dst_v = i+1;
    v->is_v = 1;
    v->data = rand()%100;
    graph.push_back(*v);
  }
  for(int i=0;i<N_E;i++) {
    e = new graph_ve_t;
    e->src_v = rand()%N_V + 1;
    do {
      e->dst_v = rand()%N_V + 1;
    } while (e->dst_v == e->src_v);
    e->is_v = 0;
    graph.push_back(*e);
  }
  
  gettimeofday(&begin,NULL);
  graphsc(graph);
  gettimeofday(&end,NULL);
  printf("time spent=%ld\n",1000000*(end.tv_sec-begin.tv_sec)+end.tv_usec-begin.tv_usec);
  for(int i=0;i<graph.size();i++){
//    graph_ve_t& ve = graph[i];
//    if(ve.is_v==1&&ve.src_v==1) BOOST_CHECK(ve.data == 0);
//    if(ve.is_v==1&&ve.src_v==2) BOOST_CHECK(ve.data == 10);
//    if(ve.is_v==1&&ve.src_v==3) BOOST_CHECK(ve.data == 70);
//   if(ve.is_v==1&&ve.src_v==4) BOOST_CHECK(ve.data == 10);
  }
}

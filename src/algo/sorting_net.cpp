#include "./shuffle_bucket.h"
#include "algo.h"
#include "cmo.h"
#include "cmo_queue.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>
int compare(int* i, int* j) {
  int res = 0;
  asm volatile (
      "mov (%1), %%eax\n\t"
      "mov (%2), %%ebx\n\t"
      "cmp %%eax, %%ebx\n\t"
      "setg %%al\n\t"
      "movl %%eax,%0\n\t"
      :  "=r" (res)
      : "r"(i),"r"(j)
      : "%eax","%ebx"
      );
  return res;
}

int compare1(int* i, int* j) {
  int res = 0;
  asm volatile (
      "mov (%1), %%eax\n\t"
      "mov (%2), %%ebx\n\t"
      "cmp %%ebx, %%eax\n\t"
      "setg %%al\n\t"
      "movl %%eax,%0\n\t"
      :  "=r" (res)
      : "r"(i),"r"(j)
      : "%eax","%ebx"
      );
  return res;
}



void swap(int swapflag, int* i, int* j) { 
  asm volatile ( 
      "movl (%0), %%eax\n\t" 
      "movl (%1), %%ebx\n\t" 
      "mov %2, %%ecx\n\t" 
      "mov %%eax, %%edx\n\t" 
      "mov %%ebx, %%esi\n\t"

      "test %%ecx, %%ecx\n\t"
      "cmovz %%ebx, %%edx\n\t"
      "movl %%edx, (%0)\n\t"
      "cmovz %%eax, %%esi\n\t"
      "movl %%esi, (%1)\n\t"
      :
      : "r"(i),"r"(j),"r"(swapflag)
      : "%eax","%ebx","%ecx","%edx","%esi"
      );
}
int64_t g_count=0;
/*
   void cas_v2m(int* i, int* j, int dir) {
   int condition = 0;
   if (dir) condition = compare1(i,j);
   else condition = compare(i,j);
   swap(condition,i,j);
   g_count++;
   }
 */

void cas_v2m(int* i, int* j, int* p, int* q) {
//void cas_v2m(int* i, int* j) { 
    int tmp_v[VALUE_SIZE];
  if (*i > *j) {
    int tmp = *i;
    *i = *j;
    *j = tmp;
       memcpy(tmp_v,p,VALUE_SIZE*4);
       memcpy(p,q,VALUE_SIZE*4);
      memcpy(q,tmp_v,VALUE_SIZE*4);
  }
  g_count++;
}


int64_t bitonicSort(int n, int start, int* array, int* values) {
//int64_t bitonicSort(int n, int start, int* array) {
  if (n==2)
    cas_v2m(&array[start],&array[start+1],&values[start*VALUE_SIZE],&values[(start+1)*VALUE_SIZE]);
    // cas_v2m(&array[start],&array[start+1],&values[0],&values[0]);
    //cas_v2m(&array[start],&array[start+1]);
  else {
    for(int i=0;i<n/2;i++) {
      cas_v2m(&array[start+i],&array[start+n/2+i],&values[(start+i)*VALUE_SIZE],&values[(start+n/2+i)*VALUE_SIZE]);
      //cas_v2m(&array[start+i],&array[start+n/2+i],&values[0],&values[0]);
      //cas_v2m(&array[start+i],&array[start+n/2+i]);
    }
    //bitonicSort(n/2,start,array,values);
    //bitonicSort(n/2,start,array,values);
    bitonicSort(n/2,start,array,values);
    bitonicSort(n/2,start+n/2,array,values);
  }
}

int64_t merger(int n, int start, int* array, int* values) {
//int64_t merger(int n, int start, int* array){
  if (n==2)
    cas_v2m(&array[start],&array[start+1],&values[start*VALUE_SIZE],&values[(start+1)*VALUE_SIZE]);
    //cas_v2m(&array[start],&array[start+1],&values[0],&array[0]);
    //cas_v2m(&array[start],&array[start+1]);
  else {
    //merger(n/2,start, array,values);
    //merger(n/2,start+n/2, array,values);
    merger(n/2,start, array,values);
    merger(n/2,start+n/2, array,values);
    for(int i=0;i<n/2;i++) {
      cas_v2m(&array[start+i],&array[start+n-1-i],&values[(start+i)*VALUE_SIZE],&values[(start+n-1-i)*VALUE_SIZE]);
      //cas_v2m(&array[start+i],&array[start+n-1-i],&values[0],&values[0]);
      //cas_v2m(&array[start+i],&array[start+n-1-i]);
    }
    //bitonicSort(n/2,start,array,values);
    //bitonicSort(n/2,start+n/2,array,values);
    bitonicSort(n/2,start,array,values);
    bitonicSort(n/2,start+n/2,array,values);
  }
  return g_count;
}


/*
   void merge(int lo, int cnt, int dir, int* list) {
   if (cnt>1) {
   int k=cnt/2;
   int i;
   for (i=lo; i<lo+k; i++) {
   cas_v2m(&list[i],&list[i+k],dir);
   }
   merge(lo, k, dir, list);
   merge(lo+k, k, dir, list);
   }
   }

   int bitonicSort(int cnt, int lo, int dir, int* list) {
   if (cnt>1) {
   int k=cnt/2;
   bitonicSort(k, lo,1, list);
   bitonicSort(k, lo+k, 0, list);
   merge(lo, cnt, dir, list);
   }
   return g_count;
   }
 */

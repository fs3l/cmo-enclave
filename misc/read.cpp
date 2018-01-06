#include <stdio.h>
struct rt {
//  int g_shadow_mem[2000000] __attribute__((aligned(4096)));
};
  int g_shadow_mem[1048576] __attribute__((aligned(4096)));
extern "C" {
void tx_abort() {
  printf("abort!\n");
}
}

void begin_tx(struct rt* p_rt) {
  __asm__ (
    "jmp end_abort_handler_%=\n\t"
    "begin_abort_handler_%=:\n\t"
    "call tx_abort\n\t"
    "end_abort_handler_%=:\n\t"
    "xbegin begin_abort_handler_%=\n\t":::);
}

void end_tx() {
  __asm__ ("xend");
}

static int search(int key,int len) {
  int low = 0;
  int high = len-1;
  int mid = 0;
  int res = 0;
  while(low<=high) {
    mid = (high-low)/2 + low;
    res = g_shadow_mem[mid];
    if (res==key) break;
    if (res < key) low = mid + 1;
    else high = mid -1;
  }
  return mid;
}


int main() {
  struct rt* a_rt = new struct rt;
  for(int i=0;i<768*1024;i++)
    g_shadow_mem[i] = i;
  int sum=0;
  int j=0;
  int res=0;
  begin_tx(a_rt);
  int res = search(1,768*1024); 
  end_tx();
  printf("%d\n",res);
  return 0;
}



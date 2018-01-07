#include <unistd.h>
#include <stdio.h>
unsigned char g_shadow_mem[80000] __attribute__((aligned(4096)));
unsigned char g_interfere_mem[80000] __attribute__((aligned(4096)));
extern "C" {
  void tx_abort() {
    printf("abort!\n");
  }
}

void begin_tx() {
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

int main() {
  for(int i=0;i<16;i++)
    g_shadow_mem[i*4096] = 5;
  begin_tx();
  for(int i=0;i<8;i++)
    g_shadow_mem[i*4096] = 5;
  end_tx();
  begin_tx();
  for(int i=8;i<16;i++)
    g_shadow_mem[i*4096] = 5;
  end_tx();
  printf("%d\n",g_shadow_mem[1000]);
  return 0;
}



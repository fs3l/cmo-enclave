#include "pao.h"
#include "utils.h"

#include <cstdlib>
#include <cstring>
#include <stdio.h>
#define OLD_ALLOC 0
#define DUMMY 0
#define META_SIZE 128
#define OB_SIZE 384
#define L1_SIZE 8192
#define LLC_SIZE 1048576

// private functions
void begin_tx(PAO_p rt);
void end_tx(PAO_p rt);
void free_array_ro(ArrayRO_p ob);
void free_array_rw(ArrayRW_p ob);

extern "C" {
void pao_tx_abort(int code);
}
#if OLD_ALLOC
int32_t cal_array_ro(int32_t offset)
{
  return (offset / 48) * 1024 + offset % 48 + 16;
}

int32_t cal_array_rw(int32_t offset)
{
  return (offset / 48) * 1024 + offset % 48 + 64;
}
#else
int32_t cal_array_ro(int32_t offset) { return offset + META_SIZE+OB_SIZE+OB_SIZE; }
int32_t cal_array_rw(int32_t offset) { return offset + META_SIZE; }
#endif

PAO_p init_pao_runtime() { return new PAO_t; }
#if DUMMY
void free_pao_runtime(PAO_p rt) {delete rt;}
#else
void free_pao_runtime(PAO_p rt)
{
  for (size_t i = 0; i < rt->arrays_ro.size(); ++i) free_array_ro(rt->arrays_ro[i]);
  for (size_t i = 0; i < rt->arrays_rw.size(); ++i) free_array_rw(rt->arrays_rw[i]);
  delete rt;
}
#endif

void pao_abort(PAO_p rt, const char *abort_msg)
{
  // TODO: check if it is in leaky sec
  end_leaky_sec(rt);
  abort_message("%s\n", abort_msg);
}
#if DUMMY
ArrayRO_p init_array_ro(PAO_p rt, const int32_t *data,
                                       int32_t len)
{
  ArrayRO_p ob = new ArrayRO_t;
  ob->rt = rt;
  ob->data = data;
  ob->len = len;
  ob->iter_pos = 0;
  return ob;
}
#else
ArrayRO_p init_array_ro(PAO_p rt, const int32_t *data,
                                       int32_t len)
{
  ArrayRO_p ob = (ArrayRO_p)(&rt->g_shadow_mem[rt->meta_pos]);
  ob->rt = rt;
  ob->data = data;
  ob->len = len;
  ob->shadow_mem_len = ob->shadow_mem_pos = ob->iter_pos = 0;
  rt->arrays_ro.push_back(ob);
#if OLD_ALLOC
  rt->meta_pos += 1024;
#else
  rt->meta_pos += 16;
#endif
  return ob;
}
#endif

#if DUMMY
ArrayRW_p init_array_rw(PAO_p rt, int32_t *data, int32_t len)
{
  ArrayRW_p ob = new ArrayRW_t;
  ob->rt = rt;
  ob->data = data;
  ob->len = len;
  ob->iter_pos = 0;
  return ob;
}
#else
ArrayRW_p init_array_rw(PAO_p rt, int32_t *data, int32_t len)
{
  ArrayRW_p ob = (ArrayRW_p)(&rt->g_shadow_mem[rt->meta_pos]);
  ob->rt = rt;
  ob->data = data;
  ob->len = len;
  ob->shadow_mem_len = ob->shadow_mem_pos = ob->iter_pos = 0;
  rt->arrays_rw.push_back(ob);
#if OLD_ALLOC
  rt->meta_pos += 1024;
#else
  rt->meta_pos += 16;
#endif
  return ob;
}
#endif

#if DUMMY
void begin_leaky_sec(PAO_p rt)
{ 
}
#else
void begin_leaky_sec(PAO_p rt)
{
//tttodo preload from disk to memory
  begin_tx(rt);
}
#endif

#if DUMMY
void end_leaky_sec(PAO_p rt) {}
#else
void end_leaky_sec(PAO_p rt)
{
  end_tx(rt);
}
#endif

#if DUMMY
int32_t max_array_ro_shadow_mem_size(PAO_p _rt, ArrayRO_p ob)
{
  return ob->len;
}
int32_t max_array_rw_shadow_mem_size(PAO_p _rt, ArrayRW_p ob)
{
  // TODO
  return ob->len;
}
#else
int32_t max_array_ro_shadow_mem_size(PAO_p _rt, ArrayRO_p ob)
{
  return min(OB_SIZE/2, ob->len - ob->shadow_mem_pos);
}
int32_t max_array_rw_shadow_mem_size(PAO_p _rt, ArrayRW_p ob)
{
  // TODO
  return min(OB_SIZE, ob->len - ob->shadow_mem_pos);
}
#endif

void begin_tx(PAO_p rt)
{
  for (size_t i = 0; i < rt->arrays_ro.size(); ++i) {
    ArrayRO_p ob = rt->arrays_ro[i];
    ob->shadow_mem_len = max_array_ro_shadow_mem_size(rt, ob);
    ob->iter_pos = 0;
    int iob = ob->shadow_mem;

    for (int i = 0; i < ob->shadow_mem_len; i++) {
      rt->g_shadow_mem[cal_array_ro(iob + i)] = ob->data[ob->shadow_mem_pos + i];
    }
  }

  for (size_t i = 0; i < rt->arrays_rw.size(); ++i) {
    ArrayRW_p ob = rt->arrays_rw[i];
    ob->shadow_mem_len = max_array_rw_shadow_mem_size(rt, ob);
    ob->iter_pos = 0;
    int iob = ob->shadow_mem;
    for (int i = 0; i < ob->shadow_mem_len; i++) {
      rt->g_shadow_mem[cal_array_rw(iob + i)] = ob->data[ob->shadow_mem_pos + i];
    }
  }

  __asm__(
      "jmp end_abort_handler_%=\n\t"
      "begin_abort_handler_%=:\n\t"
      "mov %%eax, %%edi\n\t"
      /*"push %%rax\n\t"
      "push %%rcx\n\t"
      "push %%rdx\n\t"
      "push %%rsi\n\t"
      "push %%rdi\n\t"
      "push %%r8\n\t"
      "push %%r9\n\t"
      "push %%r10\n\t"
      "push %%r11\n\t"
      "call pao_tx_abort\n\t"
      "pop %%r11\n\t"
      "pop %%r10\n\t"
      "pop %%r9\n\t"
      "pop %%r8\n\t"
      "pop %%rdi\n\t"
      "pop %%rsi\n\t"
      "pop %%rdx\n\t"
      "pop %%rcx\n\t"
      "pop %%rax\n\t"*/
      "end_abort_handler_%=:\n\t"
      "mov %0, %%rdi\n\t"
      "mov $0, %%eax\n\t"
      "mov %%rdi, %%rcx\n\t"
      "loop_ep_%=:\n\t"
      "cmpl $4200, %%eax\n\t"
     // "cmpl $0, %%eax\n\t"
      "jge endloop_ep_%=\n\t"
      "movl (%%rcx), %%r11d\n\t"
      "addl $1, %%eax\n\t"
      "add $4, %%rcx\n\t"
      "jmp loop_ep_%=\n\t"
      "endloop_ep_%=:\n\t"
      "xbegin begin_abort_handler_%=\n\t"
      "mov $0, %%eax\n\t"
      "mov %%rdi, %%rcx\n\t"
      "loop_ip_%=:\n\t"
      "cmpl $4200, %%eax\n\t"
      //"cmpl $0, %%eax\n\t"
      "jge endloop_ip_%=\n\t"
      "movl (%%rcx), %%r11d\n\t"
      "addl $1, %%eax\n\t"
      "add $4, %%rcx\n\t"
      "jmp loop_ip_%=\n\t"
      "endloop_ip_%=:\n\t"
      : /* no output */
      : "r"(rt->g_shadow_mem)
      : "%rdi", "%eax");
}

void end_tx(PAO_p rt)
{
  __asm__("xend\n\t");
  for (size_t i = 0; i < rt->arrays_ro.size(); ++i) {
    ArrayRO_p ob = rt->arrays_ro[i];
    ob->shadow_mem_pos += ob->iter_pos;
    ob->shadow_mem_len = 0;
  }

  for (size_t i = 0; i < rt->arrays_rw.size(); ++i) {
    ArrayRW_p ob = rt->arrays_rw[i];
    int iob = ob->shadow_mem;
    for (int i = 0; i < ob->shadow_mem_len; i++) {
      ob->data[ob->shadow_mem_pos + i] = rt->g_shadow_mem[cal_array_rw(iob + i)];
    }
    ob->shadow_mem_pos += ob->iter_pos;
    ob->shadow_mem_len = 0;
  }
}

#if DUMMY
void free_array_ro(ArrayRO_p ob) { }
void free_array_rw(ArrayRW_p ob) { }
#else
void free_array_ro(ArrayRO_p ob) { ob->shadow_mem = -1; }
void free_array_rw(ArrayRW_p ob) { ob->shadow_mem = -1; }
#endif

#if DUMMY
int32_t read_at(ArrayRO_p ob, int32_t idx) 
{
//return ob->data[idx];
  int res = 0;
  for (int i=0;i<ob->len;i++) {
       bool cond = (idx == i);
       cmove_int32(cond,&ob->data[idx],&res);
  }
  return res;
}
#else
int32_t read_at(ArrayRO_p ob, int32_t idx) 
{
  int32_t data = ob->g_shadow_mem[cal_array_ro(ob->shadow_mem + idx)];
  if (ob->iter_pos == ob->shadow_mem_len &&
      ob->shadow_mem_pos + ob->shadow_mem_len < ob->len) {
    end_tx(ob->rt);
    begin_tx(ob->rt);
  }
  return data;
}
#endif

#if DUMMY
void write_at(ArrayRW_p ob, int32_t idx, int32_t data) 
{
  // ob->data[idx] = data;
  for (int i=0;i<ob->len;i++) {
       bool cond = (idx == i);
       cmove_int32(cond,&data,&ob->data[idx]);
  }
}
#else
void write_at(ArrayRW_p ob, int32_t idx, int32_t data) 
{
  ob->g_shadow_mem[cal_array_rw(ob->shadow_mem + idx)] = data;
  if (ob->iter_pos == ob->shadow_mem_len &&
      ob->shadow_mem_pos + ob->shadow_mem_len < ob->len) {
    end_tx(ob->rt);
    begin_tx(ob->rt);
  }
}
#endif

void pao_tx_abort(int code) { print_message("abort! (code %d)\n", code); }

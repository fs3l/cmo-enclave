#include "cmo.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint64_t coda_context[100];
// private functions
void begin_tx(CMO_p rt);
void end_tx(CMO_p rt);
void free_read_ob(ReadObIterator_p ob);
void free_write_ob(WriteObIterator_p ob);
void free_nob(NobArray_p nob);
int32_t max_read_ob_shadow_mem_size(CMO_p rt, ReadObIterator_p ob);
int32_t max_write_ob_shadow_mem_size(CMO_p rt, WriteObIterator_p ob);
int32_t cal_ob(int32_t offset) { return (offset / 48) * 1024 + offset % 48 + 16; }
int32_t cal_ob_rw(int32_t offset)
{
  return (offset / 48) * 1024 + offset % 48 + 64;
}

int32_t cal_nob(int32_t offset)
{
  return (offset / 640) * 1024 + offset % 640 + 111;
}

CMO_p init_cmo_runtime() { 
  return new CMO_t; 
}
void free_cmo_runtime(CMO_p rt)
{
  for (size_t i = 0; i < rt->r_obs.size(); ++i) free_read_ob(rt->r_obs[i]);
  for (size_t i = 0; i < rt->w_obs.size(); ++i) free_write_ob(rt->w_obs[i]);
  for (size_t i = 0; i < rt->nobs.size(); ++i) free_nob(rt->nobs[i]);
  delete rt;
}

void cmo_abort(CMO_p rt, const char *abort_msg)
{
  // TODO: check if it is in leaky sec
  end_leaky_sec(rt);
  // TODO: replace printf and abort with ecalls
  printf("%s\n", abort_msg);
  abort();
}

ReadObIterator_p init_read_ob_iterator(CMO_p rt, const int32_t *data,
    int32_t len)
{
  ReadObIterator_p ob = (ReadObIterator_p)(&rt->g_shadow_mem[rt->meta_pos]);
  ob->rt = rt;
  ob->data = data;
  ob->len = len;
  ob->shadow_mem_len = ob->shadow_mem_pos = ob->iter_pos = 0;
  rt->r_obs.push_back(ob);
  rt->meta_pos+=1024;
  return ob;
}

WriteObIterator_p init_write_ob_iterator(CMO_p rt, int32_t *data, int32_t len)
{
  WriteObIterator_p ob = (WriteObIterator_p)(&rt->g_shadow_mem[rt->meta_pos]);
  ob->rt = rt;
  ob->data = data;
  ob->len = len;
  ob->shadow_mem_len = ob->shadow_mem_pos = ob->iter_pos = 0;
  rt->w_obs.push_back(ob);
  rt->meta_pos+=1024;
  return ob;
}

NobArray_p init_nob_array(CMO_p rt, int32_t *data, int32_t len)
{
  NobArray_p nob = (NobArray_p)(&rt->g_shadow_mem[rt->meta_pos]);
  nob->rt = rt;
  nob->data = data;
  nob->len = len;
  rt->nobs.push_back(nob);
  rt->meta_pos+=1024;
  return nob;
}

void begin_leaky_sec(CMO_p rt) {
  int32_t len_sum = 0;
  for (size_t i = 0; i < rt->nobs.size(); ++i) {
    NobArray_p nob = rt->nobs[i];
    nob->shadow_mem = rt->cur_nob;
    nob->g_shadow_mem = rt->g_shadow_mem;
    rt->cur_nob+=nob->len;
    //TODO cache size dynamically check
    len_sum+=nob->len;
    if(len_sum>8192)
      abort();
  }

  for (size_t i=0; i<rt->r_obs.size();++i) {
    ReadObIterator_p ob = rt->r_obs[i];
    ob->shadow_mem = rt->cur_ob;
    ob->g_shadow_mem = rt->g_shadow_mem;
    //rt->cur_ob+=ob->len;
    rt->cur_ob+=48;
  }

  for (size_t i=0; i<rt->w_obs.size();++i) {
    WriteObIterator_p ob = rt->w_obs[i];
    ob->shadow_mem = rt->cur_ob_rw;
    ob->g_shadow_mem = rt->g_shadow_mem;
    //rt->cur_ob_rw+=ob->len;
    rt->cur_ob+=48;
  }


  for (size_t i = 0; i < rt->nobs.size(); ++i) {
    NobArray_p nob = rt->nobs[i];
    int inob = nob->shadow_mem;
    for(int i=0;i<nob->len;i++) {
      rt->g_shadow_mem[cal_nob(inob+i)] = nob->data[i];
    }
  }
  begin_tx(rt); 
}

void end_leaky_sec(CMO_p rt) { 
  end_tx(rt);
  for (size_t i = 0; i < rt->nobs.size(); ++i) {
    NobArray_p nob = rt->nobs[i];
    int inob = nob->shadow_mem;
    for(int i=0;i<nob->len;i++) {
      nob->data[i] = rt->g_shadow_mem[cal_nob(inob+i)];
    }
  }
  rt->meta_pos=0;
  rt->cur_ob=0;
  rt->cur_ob_rw=0;
  rt->cur_nob=0;
}
int32_t max_read_ob_shadow_mem_size(CMO_p _rt, ReadObIterator_p ob)
{
  return min(48, ob->len - ob->shadow_mem_pos);
}
int32_t max_write_ob_shadow_mem_size(CMO_p _rt, WriteObIterator_p ob)
{
  //TODO
  return min(48, ob->len - ob->shadow_mem_pos);
}

void begin_tx(CMO_p rt)
{
  for (size_t i = 0; i < rt->r_obs.size(); ++i) {
    ReadObIterator_p ob = rt->r_obs[i];
    ob->shadow_mem_len = max_read_ob_shadow_mem_size(rt, ob);
    ob->iter_pos = 0;
    int iob = ob->shadow_mem;

    //TODO REMOVE memory copy here!!! 
    for(int i=0;i<ob->shadow_mem_len;i++) {
      rt->g_shadow_mem[cal_ob(iob+i)] = ob->data[ob->shadow_mem_pos+i];
    }
  }

  for (size_t i = 0; i < rt->w_obs.size(); ++i) {
    WriteObIterator_p ob = rt->w_obs[i];
    ob->shadow_mem_len = max_write_ob_shadow_mem_size(rt, ob);
    ob->iter_pos = 0;
    int iob = ob->shadow_mem;
    for(int i=0;i<ob->shadow_mem_len;i++) {
      rt->g_shadow_mem[cal_ob_rw(iob+i)] = ob->data[ob->shadow_mem_pos+i];
    }
  }

  __asm__(
      "mov %%rax,%0\n\t"
      "mov %%rbx,%1\n\t"
      "mov %%rcx,%2\n\t"
      "mov %%rdx,%3\n\t"
      "mov %%rdi,%4\n\t"
      "mov %%rsi,%5\n\t"
      "mov %%r8,%6\n\t"
      "mov %%r9,%7\n\t"
      "mov %%r10,%8\n\t"
      "mov %%r11,%9\n\t"
      "mov %%r12,%10\n\t"
      "mov %%r13,%11\n\t"
      "mov %%r15,%12\n\t"
      "mov %13,%%r15\n\t"
      "lea (%%rip),%%r14\n\t"
      : "=r"(coda_context[0]), "=r"(coda_context[1]), "=r"(coda_context[2]),
      "=r"(coda_context[3]), "=r"(coda_context[4]), "=r"(coda_context[5]),
      "=r"(coda_context[6]), "=r"(coda_context[7]), "=r"(coda_context[8]),
      "=r"(coda_context[9]), "=r"(coda_context[10]), "=r"(coda_context[11]),
      "=r"(coda_context[12])
        : "r"(&coda_context[0])
               :"%r15","%r14");

  __asm__("mov %0,%%rdi\n\t" : : "r"(rt->g_shadow_mem) : "%rdi");
  __asm__(
      "mov $0, %%eax\n\t"
      "mov %%rdi, %%rcx\n\t"
      "loop_ep_%=:\n\t"
      "cmpl  $7690,%%eax\n\t"
      "jge    endloop_ep_%=\n\t"
      "movl   (%%rcx),%%r11d\n\t"
      "addl   $1, %%eax\n\t"
      "add    $4, %%rcx\n\t"
      "jmp    loop_ep_%=\n\t"
      "endloop_ep_%=:\n\t"
      //"xbegin coda_abort_handler\n\t"
      "mov $0, %%eax\n\t"
      "mov %%rdi, %%rcx\n\t"
      "loop_ip_%=:\n\t"
      "cmpl  $7690,%%eax\n\t"
      "jge    endloop_ip_%=\n\t"
      "movl   (%%rcx),%%r11d\n\t"
      "addl   $1, %%eax\n\t"
      "add    $4, %%rcx\n\t"
      "jmp    loop_ip_%=\n\t"
      "endloop_ip_%=:\n\t"
      ::
      :);
}

void end_tx(CMO_p rt)
{
  //__asm__("xend\n\t");
  for (size_t i = 0; i < rt->r_obs.size(); ++i) {
    ReadObIterator_p ob = rt->r_obs[i];
    ob->shadow_mem_pos += ob->iter_pos;
    ob->shadow_mem_len = 0;
  }

  for (size_t i = 0; i < rt->w_obs.size(); ++i) {
    WriteObIterator_p ob = rt->w_obs[i];
    int iob = ob->shadow_mem;
    for(int i=0;i<ob->shadow_mem_len;i++) {
      ob->data[ob->shadow_mem_pos+i] = rt->g_shadow_mem[cal_ob_rw(iob+i)];
    }
    ob->shadow_mem_pos += ob->iter_pos;
    ob->shadow_mem_len = 0;
  }
}

void free_read_ob(ReadObIterator_p ob)
{
  ob->shadow_mem = -1;
}

void free_write_ob(WriteObIterator_p ob)
{
  ob->shadow_mem = -1;
}

void free_nob(NobArray_p nob)
{
  nob->shadow_mem = -1; 
}

int32_t ob_read_next(ReadObIterator_p ob)
{
  int32_t data = ob->g_shadow_mem[cal_ob(ob->shadow_mem+ob->iter_pos++)];
  if (ob->iter_pos == ob->shadow_mem_len &&
      ob->shadow_mem_pos + ob->shadow_mem_len < ob->len) {
    end_tx(ob->rt);
    begin_tx(ob->rt);
  }
  return data;
}

void ob_write_next(WriteObIterator_p ob, int32_t data)
{
  ob->g_shadow_mem[cal_ob_rw(ob->shadow_mem+ob->iter_pos++)] = data;
  if (ob->iter_pos == ob->shadow_mem_len &&
      ob->shadow_mem_pos + ob->shadow_mem_len < ob->len) {
    end_tx(ob->rt);
    begin_tx(ob->rt);
  }
}

void reset_read_ob(ReadObIterator_p ob) { ob->shadow_mem_pos = 0; }
void reset_write_ob(WriteObIterator_p ob) { ob->shadow_mem_pos = 0; }
int32_t nob_read_at(const NobArray_p nob, int32_t addr)
{
  return  nob->g_shadow_mem[cal_nob(nob->shadow_mem+addr)];
}
void nob_write_at(NobArray_p nob, int32_t addr, int32_t data)
{
  nob->g_shadow_mem[cal_nob(nob->shadow_mem+addr)] = data;
}

extern "C" {
  void cmo_tx_abort(int code)
  {
    printf("abort!\n");
  }
}

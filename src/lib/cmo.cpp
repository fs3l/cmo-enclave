#include "cmo.h"
#include "utils.h"

#include <cstdlib>
#include <cstring>
#include <stdio.h>
#define OLD_ALLOC 1
#define DUMMY 1
#define META_SIZE 128
#define OB_SIZE 384
#define L1_SIZE 8192
#define LLC_SIZE 1048576
#define LINE_SIZE 16
#define SET_SIZE 128
#define ACTIVE_SET_SIZE 80
#define L1_SET 64
#define META_SET 1
#define OB_RW_SIZE ACTIVE_SET_SIZE * 6
#define OB_R_SIZE ACTIVE_SET_SIZE * 6
uint32_t shadow_mem[2 * 1024 * 1024] __attribute__((aligned(4096)));
// private functions
void begin_tx(CMO_p rt);
void end_tx(CMO_p rt);
void free_read_ob(ReadObIterator_p ob);
void free_write_ob(WriteObIterator_p ob);
void free_nob(NobArray_p nob);
void free_read_nob(ReadNobArray_p nob);
int32_t max_read_ob_shadow_mem_size(CMO_p rt, ReadObIterator_p ob);
int32_t max_write_ob_shadow_mem_size(CMO_p rt, WriteObIterator_p ob);

extern "C" {
  void cmo_tx_abort(int code);
}
#if PFO
uint16_t sets[64*8];
//return 0 if set_number contains tag, otherwise returns current #distinct tag in one set 
int32_t check_tag(uint32_t set_number, uint16_t tag,uint32_t *g_mem) {
  uint32_t idx = set_number * 4;
  idx = (idx/48)*1024 + idx%48 +16;
  uint16_t* set_member =  (uint16_t*)&g_mem[idx];
  //uint16_t* set_member =  (uint16_t*)&sets[set_number*8];
  //for(int i=0;i<8;i++)
  //  printf("%d,",set_member[i]);
  //printf("\n and tag=%d\n",tag);
  for(int i=0;i<8;i++) {
    if (set_member[i] == tag) return 0;
    if (set_member[i] == 0) {
      set_member[i] = tag; 
      return i+1;
    }
  }
  return 9;
}

void clear_tag(uint32_t* g_mem) {
  int idx = 0;
  void* set_member;
  for(int i=0;i<64;i++) {
    i = i*4;
    idx = (i/48)*1024+i%48 + 16;
    set_member = (void*)&g_mem[idx];
    //set_member = (void*)&sets[i*8];
    memset(set_member,0,16);
  }
}
#endif

#if OLD_ALLOC
int32_t cal_ob(int32_t offset, ALLOC_p alloc)
{
  return (offset / (alloc->ob_r*LINE_SIZE)) * 1024 + offset % (alloc->ob_r*LINE_SIZE) + alloc->meta*LINE_SIZE;
}

int32_t cal_ob_rw(int32_t offset, ALLOC_p alloc)
{
  return (offset / (alloc->ob_w*LINE_SIZE)) * 1024 + offset % (alloc->ob_w*LINE_SIZE) + (alloc->meta + alloc->ob_r)*LINE_SIZE;
}

int32_t cal_nob(int32_t offset, ALLOC_p alloc)
{
  return (offset / (alloc->nob_w*LINE_SIZE)) * 1024 + offset % (alloc->nob_w*LINE_SIZE) + (alloc->meta+alloc->ob_r+alloc->ob_w)*LINE_SIZE;
}


int32_t cal_read_nob(int32_t offset, ALLOC_p alloc) { 
  return (offset / (alloc->nob_r*LINE_SIZE)) * 1024 + offset % (alloc->nob_r*LINE_SIZE) + (alloc->meta+alloc->ob_r+alloc->ob_w+alloc->nob_w)*LINE_SIZE;
}
#else
int32_t cal_ob(int32_t offset) { return offset + META_SIZE; }
int32_t cal_ob_rw(int32_t offset) { return offset + META_SIZE+OB_R_SIZE; }
int32_t cal_nob(int32_t offset) { return offset + META_SIZE+OB_R_SIZE+OB_RW_SIZE; }
int32_t cal_read_nob(int32_t offset) { return offset + META_SIZE; }
#endif

CMO_p init_cmo_runtime() { 
  CMO_p a = new CMO_t; 
  a->g_shadow_mem = shadow_mem; 
  a->cur_ob = 0;
  a->meta_pos = 0;
  a->cur_ob_rw = 0;
  a->cur_nob = 0;
  for(int i=0;i<2*1024*1024;i++) 
    shadow_mem[i] = 0;
  return a; 
}
#if DUMMY
void free_cmo_runtime(CMO_p rt) {delete rt;}
#else
void free_cmo_runtime(CMO_p rt)
{
  for (size_t i = 0; i < rt->r_obs.size(); ++i) free_read_ob(rt->r_obs[i]);
  for (size_t i = 0; i < rt->w_obs.size(); ++i) free_write_ob(rt->w_obs[i]);
  for (size_t i = 0; i < rt->nobs.size(); ++i) free_nob(rt->nobs[i]);
  for (size_t i = 0; i < rt->r_nobs.size(); ++i) free_read_nob(rt->r_nobs[i]);
  delete rt;
}
#endif

void cmo_abort(CMO_p rt, const char *abort_msg)
{
  // TODO: check if it is in leaky sec
  end_leaky_sec(rt);
  abort_message("%s\n", abort_msg);
}
#if DUMMY
ReadObIterator_p init_read_ob_iterator(CMO_p rt, const int32_t *data,
    int32_t len)
{
  ReadObIterator_p ob = new ReadObIterator_t;
  ob->rt = rt;
  ob->data = data;
  ob->len = len;
  ob->iter_pos = 0;
  return ob;
}
#else
ReadObIterator_p init_read_ob_iterator(CMO_p rt, const int32_t *data,
    int32_t len)
{
  ReadObIterator_p ob = (ReadObIterator_p)(&rt->g_shadow_mem[rt->meta_pos]);
  ob->rt = rt;
  ob->data = data;
  ob->len = len;
  ob->shadow_mem_len = ob->shadow_mem_pos = ob->iter_pos = 0;
  rt->r_obs.push_back(ob);
  ob->alloc = (ALLOC_p)(&rt->g_shadow_mem[1024*6]); 
#if OLD_ALLOC
  rt->meta_pos += 1024;
#else
  rt->meta_pos += 16;
#endif
  return ob;
}
#endif

#if DUMMY
WriteObIterator_p init_write_ob_iterator(CMO_p rt, int32_t *data, int32_t len)
{
  WriteObIterator_p ob = new WriteObIterator_t;
  ob->rt = rt;
  ob->data = data;
  ob->len = len;
  ob->iter_pos = 0;
  return ob;
}
#else
WriteObIterator_p init_write_ob_iterator(CMO_p rt, int32_t *data, int32_t len)
{
  WriteObIterator_p ob = (WriteObIterator_p)(&rt->g_shadow_mem[rt->meta_pos]);
  ob->rt = rt;
  ob->data = data;
  ob->len = len;
  ob->alloc = (ALLOC_p)(&rt->g_shadow_mem[1024*6]); 
  ob->shadow_mem_len = ob->shadow_mem_pos = ob->iter_pos = 0;
  rt->w_obs.push_back(ob);
#if OLD_ALLOC
  rt->meta_pos += 1024;
#else
  rt->meta_pos += 16;
#endif
  return ob;
}
#endif

#if DUMMY
NobArray_p init_nob_array(CMO_p rt, int32_t *data, int32_t len)
{
  NobArray_p nob = new NobArray_t;
  nob->rt=rt;
  nob->data = data;
  nob->len = len;
  return nob;
}
#else
NobArray_p init_nob_array(CMO_p rt, int32_t *data, int32_t len)
{
  NobArray_p nob = (NobArray_p)(&rt->g_shadow_mem[rt->meta_pos]);
  nob->rt = rt;
  nob->data = data;
  nob->alloc = (ALLOC_p)(&rt->g_shadow_mem[1024*6]); 
  nob->len = len;
  rt->nobs.push_back(nob);
#if OLD_ALLOC
  rt->meta_pos += 1024;
#else
  rt->meta_pos += 16;
#endif
  return nob;
}
#endif

#if DUMMY
ReadNobArray_p init_read_nob_array(CMO_p rt, int32_t *data, int32_t len)
{
  ReadNobArray_p nob = new ReadNobArray_t;
  nob->rt = rt;
  nob->data= data;
  nob->len = len;
  return nob;
}
#else
ReadNobArray_p init_read_nob_array(CMO_p rt, int32_t *data, int32_t len)
{
  // TODO
  ReadNobArray_p nob = (ReadNobArray_p)(&rt->g_shadow_mem[rt->meta_pos]);
  nob->rt = rt;
  nob->data = data;
  nob->len = len;
  nob->alloc = (ALLOC_p)(&rt->g_shadow_mem[1024*6]); 
  rt->r_nobs.push_back(nob);
#if OLD_ALLOC
  rt->meta_pos += 1024;
#else
  rt->meta_pos += 16;
#endif
  return nob;
}
#endif

#if DUMMY
void begin_leaky_sec(CMO_p rt)
{ 
}
#else
#if PFO
void begin_leaky_sec(CMO_p rt)
{
  ALLOC_p alloc = (ALLOC_p)(&rt->g_shadow_mem[1024*6]);
  alloc->meta = 4;
  int32_t len_sum = 0;
  for (size_t i = 0; i < rt->nobs.size(); ++i) {
    NobArray_p nob = rt->nobs[i];
    nob->shadow_mem = rt->cur_nob;
    nob->g_shadow_mem = rt->g_shadow_mem;
    rt->cur_nob += nob->len;
    len_sum += nob->len;
  }
  //  printf("nob_w size=%d and nob_w count=%d\n",len_sum,rt->nobs.size()); 

  //alloc->nob_w = len_sum/ACTIVE_SET_SIZE + 1;
  alloc->nob_w = 40;
  len_sum = 0;
  for (size_t i = 0; i < rt->r_nobs.size(); ++i) {
    ReadNobArray_p nob = rt->r_nobs[i];
    nob->shadow_mem = rt->cur_nob;
    nob->g_shadow_mem = rt->g_shadow_mem;
    rt->cur_nob += nob->len;
    len_sum += nob->len;
  }
  //  printf("nob_r size=%d and nob_r count=%d\n",len_sum,rt->r_nobs.size()); 
  alloc->nob_r = len_sum/(1024*24) + 1;
  //printf("nob_w=%d,nob_r=%d,ob_w=%d,ob_r=%d\n",alloc->nob_w,alloc->nob_r,alloc->ob_w,alloc->ob_r);
  //  printf("ob_w size=%d and ob_w count=%d\n",len_sum,rt->w_obs.size()); 

  alloc->ob_w = 1;
  alloc->ob_r = 1; //tttodo more sophisticated policy to preset $sets for ob_r
  for (size_t i = 0; i < rt->r_obs.size(); ++i) {
    ReadObIterator_p ob = rt->r_obs[i];
    ob->shadow_mem = rt->cur_ob;
    ob->g_shadow_mem = rt->g_shadow_mem;
    rt->cur_ob += ((alloc->ob_r)*ACTIVE_SET_SIZE)/rt->r_obs.size();
  }

  for (size_t i = 0; i < rt->w_obs.size(); ++i) {
    WriteObIterator_p ob = rt->w_obs[i];
    ob->shadow_mem = rt->cur_ob_rw;
    ob->g_shadow_mem = rt->g_shadow_mem;
    rt->cur_ob_rw += ((alloc->ob_w)*ACTIVE_SET_SIZE)/rt->w_obs.size();
  }

  for (size_t i = 0; i < rt->nobs.size(); ++i) {
    NobArray_p nob = rt->nobs[i];
    int inob = nob->shadow_mem;
    for (int i = 0; i < nob->len; i++) {
      rt->g_shadow_mem[cal_nob(inob + i,nob->alloc)] = nob->data[i];
    }
  }

  for (size_t i = 0; i < rt->r_nobs.size(); ++i) {
    ReadNobArray_p nob = rt->r_nobs[i];
    int inob = nob->shadow_mem;
    for (int i = 0; i < nob->len; i++) {
      rt->g_shadow_mem[cal_read_nob(inob + i,nob->alloc)] = nob->data[i];
    }
  }

  begin_tx(rt);
}
#else
void begin_leaky_sec(CMO_p rt)
{
  //printf("start begin_leaky\n");
  int available_set = 64;
  int available_llc = LLC_SIZE;
  available_set--; //for meta
  ALLOC_p alloc = (ALLOC_p)(&rt->g_shadow_mem[1024*6]);
  alloc->meta = 1;
  int32_t len_sum = 0;
  for (size_t i = 0; i < rt->nobs.size(); ++i) {
    NobArray_p nob = rt->nobs[i];
    nob->shadow_mem = rt->cur_nob;
    nob->g_shadow_mem = rt->g_shadow_mem;
    rt->cur_nob += nob->len;
    len_sum += nob->len;
  }
  //printf("nob_w size=%d and nob_w count=%d\n",len_sum,rt->nobs.size()); 

  alloc->nob_w = len_sum/ACTIVE_SET_SIZE + 1;
  if (len_sum > available_llc || alloc->nob_w > available_set) abort_message("nob_w size\n"); 
  available_set -= alloc->nob_w; 
  available_llc -= len_sum;

  len_sum = 0;
  for (size_t i = 0; i < rt->r_nobs.size(); ++i) {
    ReadNobArray_p nob = rt->r_nobs[i];
    nob->shadow_mem = rt->cur_nob;
    nob->g_shadow_mem = rt->g_shadow_mem;
    rt->cur_nob += nob->len;
    len_sum += nob->len;
  }
  //printf("nob_r size=%d and nob_r count=%d\n",len_sum,rt->r_nobs.size()); 
  alloc->nob_r = len_sum/(1024*24) + 1;
  if (len_sum > available_llc || alloc->nob_r > available_set) abort_message("nob_r size\n"); 
  available_set -= alloc->nob_r;
  available_llc -= len_sum;

  int len_sum_r = 0;
  for (size_t i = 0; i < rt->r_obs.size(); ++i) {
    ReadObIterator_p ob = rt->r_obs[i];
    len_sum_r += ob->len;
    ob->g_shadow_mem = rt->g_shadow_mem;
  }
  //printf("ob_r size=%d and ob_r count=%d\n",len_sum_r,rt->r_obs.size()); 

  len_sum = 0;
  for (size_t i = 0; i < rt->w_obs.size(); ++i) {
    WriteObIterator_p ob = rt->w_obs[i];
    ob->g_shadow_mem = rt->g_shadow_mem;
    len_sum += ob->len;
  }
  //printf("ob_w size=%d and ob_w count=%d\n",len_sum,rt->w_obs.size()); 

  if (len_sum!=0 || len_sum_r!=0) {
    alloc->ob_w = (available_set)*(len_sum)/(len_sum+len_sum_r);
    alloc->ob_r = available_set -= alloc->ob_w;
    for (size_t i = 0; i < rt->r_obs.size(); ++i) {
      ReadObIterator_p ob = rt->r_obs[i];
      ob->shadow_mem = rt->cur_ob;
      rt->cur_ob += ((alloc->ob_r)*ACTIVE_SET_SIZE)/rt->r_obs.size();
    }}

  for (size_t i = 0; i < rt->w_obs.size(); ++i) {
    WriteObIterator_p ob = rt->w_obs[i];
    ob->shadow_mem = rt->cur_ob_rw;
    rt->cur_ob_rw += ((alloc->ob_w)*ACTIVE_SET_SIZE)/rt->w_obs.size();
  }

  for (size_t i = 0; i < rt->nobs.size(); ++i) {
    NobArray_p nob = rt->nobs[i];
    int inob = nob->shadow_mem;
    for (int i = 0; i < nob->len; i++) {
      rt->g_shadow_mem[cal_nob(inob + i,nob->alloc)] = nob->data[i];
    }
  }

  for (size_t i = 0; i < rt->r_nobs.size(); ++i) {
    ReadNobArray_p nob = rt->r_nobs[i];
    int inob = nob->shadow_mem;
    for (int i = 0; i < nob->len; i++) {
      rt->g_shadow_mem[cal_read_nob(inob + i,nob->alloc)] = nob->data[i];
    }
  }
  begin_tx(rt);
}
#endif
#endif

#if DUMMY
void end_leaky_sec(CMO_p rt) {}
#else
void end_leaky_sec(CMO_p rt)
{
  end_tx(rt);
  for (size_t i = 0; i < rt->nobs.size(); ++i) {
    NobArray_p nob = rt->nobs[i];
    int inob = nob->shadow_mem;
    for (int i = 0; i < nob->len; i++) {
      nob->data[i] = rt->g_shadow_mem[cal_nob(inob + i,nob->alloc)];
    }
  }
  rt->meta_pos = 0;
  rt->cur_ob = 0;
  rt->cur_ob_rw = 0;
  rt->cur_nob = 0;
}
#endif

#if DUMMY
int32_t max_read_ob_shadow_mem_size(CMO_p _rt, ReadObIterator_p ob)
{
  return ob->len;
}
int32_t max_write_ob_shadow_mem_size(CMO_p _rt, WriteObIterator_p ob)
{
  // TODO
  return ob->len;
}
#else
int32_t max_read_ob_shadow_mem_size(CMO_p _rt, ReadObIterator_p ob)
{
  return min((int)((ob->alloc->ob_r*ACTIVE_SET_SIZE)/(_rt->r_obs.size())), ob->len - ob->shadow_mem_pos);
}
int32_t max_write_ob_shadow_mem_size(CMO_p _rt, WriteObIterator_p ob)
{
  // TODO
  return min((int)((ob->alloc->ob_w*ACTIVE_SET_SIZE)/(_rt->w_obs.size())), ob->len - ob->shadow_mem_pos);
}
#endif
#if PFO
void begin_tx_pfo(CMO_p rt)
{

  __asm__(
      "jmp end_abort_handler_%=\n\t"
      "begin_abort_handler_%=:\n\t"
      "end_abort_handler_%=:\n\t"
      "xbegin begin_abort_handler_%=\n\t"
      :::);
}
#endif
void begin_tx(CMO_p rt)
{
  for (size_t i = 0; i < rt->r_obs.size(); ++i) {
    ReadObIterator_p ob = rt->r_obs[i];
    ob->shadow_mem_len = max_read_ob_shadow_mem_size(rt, ob);
    ob->iter_pos = 0;
    int iob = ob->shadow_mem;
    // TODO REMOVE memory copy here!!!
    for (int i = 0; i < ob->shadow_mem_len; i++) {
      rt->g_shadow_mem[cal_ob(iob + i,ob->alloc)] = ob->data[ob->shadow_mem_pos + i];
    }
  }

  for (size_t i = 0; i < rt->w_obs.size(); ++i) {
    WriteObIterator_p ob = rt->w_obs[i];
    ob->shadow_mem_len = max_write_ob_shadow_mem_size(rt, ob);
    ob->iter_pos = 0;
    int iob = ob->shadow_mem;
    for (int i = 0; i < ob->shadow_mem_len; i++) {
      rt->g_shadow_mem[cal_ob_rw(iob + i,ob->alloc)] = ob->data[ob->shadow_mem_pos + i];
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
        "call cmo_tx_abort\n\t"
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
    "cmpl $0, %%eax\n\t"
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
    "cmpl $0, %%eax\n\t"
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
#if PFO
void end_tx_pfo(CMO_p rt) {
  __asm__("xend\n\t");
  clear_tag(rt->g_shadow_mem);
}
#endif
void end_tx(CMO_p rt)
{
  __asm__("xend\n\t");
  for (size_t i = 0; i < rt->r_obs.size(); ++i) {
    ReadObIterator_p ob = rt->r_obs[i];
    ob->shadow_mem_pos += ob->iter_pos;
    ob->shadow_mem_len = 0;
  }

  for (size_t i = 0; i < rt->w_obs.size(); ++i) {
    WriteObIterator_p ob = rt->w_obs[i];
    int iob = ob->shadow_mem;
    for (int i = 0; i < ob->shadow_mem_len; i++) {
      ob->data[ob->shadow_mem_pos + i] = rt->g_shadow_mem[cal_ob_rw(iob + i,ob->alloc)];
    }
    ob->shadow_mem_pos += ob->iter_pos;
    ob->shadow_mem_len = 0;
  }
#if PFO
  clear_tag(rt->g_shadow_mem);
#endif
}

#if DUMMY
void free_read_ob(ReadObIterator_p ob) { }
void free_write_ob(WriteObIterator_p ob) { }
void free_nob(NobArray_p nob) { }
void free_read_nob(ReadNobArray_p nob) { }
#else
void free_read_ob(ReadObIterator_p ob) { ob->shadow_mem = -1; }
void free_write_ob(WriteObIterator_p ob) { ob->shadow_mem = -1; }
void free_nob(NobArray_p nob) { nob->shadow_mem = -1; }
void free_read_nob(ReadNobArray_p nob) { nob->shadow_mem = -1; }
#endif

#if DUMMY
int32_t ob_read_next(ReadObIterator_p ob) {return ob->data[ob->iter_pos++];}
#else
int32_t ob_read_next(ReadObIterator_p ob)
{
  int32_t data = ob->g_shadow_mem[cal_ob(ob->shadow_mem + ob->iter_pos++,ob->alloc)];
  if (ob->iter_pos == ob->shadow_mem_len &&
      ob->shadow_mem_pos + ob->shadow_mem_len < ob->len) {
    end_tx(ob->rt);
    begin_tx(ob->rt);
  }
  return data;
}
#endif

#if DUMMY
void ob_write_next(WriteObIterator_p ob, int32_t data) {ob->data[ob->iter_pos++] = data;}
#else
void ob_write_next(WriteObIterator_p ob, int32_t data)
{
  ob->g_shadow_mem[cal_ob_rw(ob->shadow_mem + ob->iter_pos++,ob->alloc)] = data;
  if (ob->iter_pos == ob->shadow_mem_len &&
      ob->shadow_mem_pos + ob->shadow_mem_len < ob->len) {
    end_tx(ob->rt);
    begin_tx(ob->rt);
  }
}
#endif

#if DUMMY
void reset_read_ob(ReadObIterator_p ob) {  }
void reset_write_ob(WriteObIterator_p ob) { }
#else
void reset_read_ob(ReadObIterator_p ob) { ob->shadow_mem_pos = 0; }
void reset_write_ob(WriteObIterator_p ob) { ob->shadow_mem_pos = 0; }
#endif

#if DUMMY
int32_t nob_read_at(const NobArray_p nob, int32_t addr)
{
  int res = 0;
  for (int i=0;i<nob->len;i++) {
    bool cond = (addr == i);
    cmove_int32(cond,&nob->data[addr],&res);
  }
  return res;
}
void nob_write_at(NobArray_p nob, int32_t addr, int32_t data)
{
  for (int i=0;i<nob->len;i++) {
    bool cond = (addr == i);
    cmove_int32(cond,&data,&nob->data[addr]);
  }
}

int32_t nob_read_at(const ReadNobArray_p nob, int32_t addr)
{
  int res = 0;
  for (int i=0;i<nob->len;i++) {
    bool cond = (addr == i);
    cmove_int32(cond,&nob->data[addr],&res);
  }
  return res;
}
#else
bool check_cache_full()
{
}
uint32_t minset = 100;
uint32_t maxset = 0;
int32_t nob_read_at(const NobArray_p nob, int32_t addr)
{
#if PFO
  uint64_t vm_addr =(uint64_t)nob->g_shadow_mem + cal_nob(nob->shadow_mem + addr,nob->alloc)*sizeof(int32_t);
  uint32_t set_idx = (vm_addr >> 6) & 0x3f;
  uint16_t tag = (vm_addr>>12) & 0xffff;
  int32_t res = 0;
  //if (set_idx<minset) minset = set_idx;
  //if (set_idx>maxset) maxset = set_idx;
  //printf("nob_read_at g_shadowm_mem=%lx,set_idx=%d, minset=%d, and maxset=%d, and cal_nob=%d and vm_addr=%lx\n",nob->g_shadow_mem,set_idx,minset,maxset,cal_nob(nob->shadow_mem+addr,nob->alloc),vm_addr);
  res = check_tag(set_idx,tag,nob->g_shadow_mem);
  //if (set_idx==32)
  //printf("calling check_tag with set=%d, tag=%d, and res=%d\n",set_idx,tag,res);
  if(res > L1_WAYS){
    //printf("nob partition\n");
    end_tx_pfo(nob->rt);
    begin_tx_pfo(nob->rt);
    check_tag(set_idx,tag,nob->g_shadow_mem);
    return nob->g_shadow_mem[cal_nob(nob->shadow_mem + addr,nob->alloc)];
  } else {
    return nob->g_shadow_mem[cal_nob(nob->shadow_mem + addr,nob->alloc)];
  }
#else
  return nob->g_shadow_mem[cal_nob(nob->shadow_mem + addr,nob->alloc)];
#endif
}

void nob_write_at(NobArray_p nob, int32_t addr, int32_t data)
{
#if PFO
  uint64_t vm_addr = (uint64_t)nob->g_shadow_mem + cal_nob(nob->shadow_mem + addr,nob->alloc)*sizeof(int32_t);
  uint32_t set_idx = (vm_addr >> 6) & 0x3f;
  uint16_t tag = (vm_addr>>12) & 0xffff;
  uint32_t res = 0;
  // if (set_idx<minset) minset = set_idx;
  // if (set_idx>maxset) maxset = set_idx;
  //printf("nob_write_at set_idx=%d, minset=%d, and maxset=%d, and cal_nob=%d and vm_addr=%lx\n",set_idx,minset,maxset,cal_nob(nob->shadow_mem+addr,nob->alloc),vm_addr);
  res = check_tag(set_idx,tag,nob->g_shadow_mem);
  //if (set_idx==32)
  //printf("calling check_tag with set=%d, tag=%d, and res=%d\n",set_idx,tag,res);
  if(res > L1_WAYS){
    //   printf("nob partition\n");
    end_tx_pfo(nob->rt);
    begin_tx_pfo(nob->rt);
    check_tag(set_idx,tag,nob->g_shadow_mem);
    nob->g_shadow_mem[cal_nob(nob->shadow_mem + addr,nob->alloc)] = data;
  } else {
    nob->g_shadow_mem[cal_nob(nob->shadow_mem + addr,nob->alloc)] = data;
  }
#else
  nob->g_shadow_mem[cal_nob(nob->shadow_mem + addr,nob->alloc)] = data;
#endif
}

int32_t nob_read_at(const ReadNobArray_p nob, int32_t addr)
{
  return nob->g_shadow_mem[cal_read_nob(nob->shadow_mem + addr,nob->alloc)];
}
#endif
void cmo_tx_abort(int code) { print_message("abort! (code %d)\n", code); }

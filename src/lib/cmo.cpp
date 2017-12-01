#include "cmo.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// private functions
void begin_tx(CMO_p rt);
void end_tx(CMO_p rt);
void free_read_ob(ReadObIterator_p ob);
void free_write_ob(WriteObIterator_p ob);
void free_nob(NobArray_p nob);
int32_t max_read_ob_shadow_mem_size(CMO_p rt, ReadObIterator_p ob);
int32_t max_write_ob_shadow_mem_size(CMO_p rt, WriteObIterator_p ob);
int32_t cal_ob(int32_t offset) { return (offset / 16) * 1024 + offset % 16 + 16; }
int32_t cal_ob_rw(int32_t offset)
{
  return (offset / 48) * 1024 + offset % 48 + 32;
}

int32_t cal_nob(int32_t offset)
{
  return (offset / 640) * 1024 + offset % 640 + 80;
}
FILE *fp;
CMO_p init_cmo_runtime() { fp = fopen("/home/ju/log","w+");return new CMO_t; }
void free_cmo_runtime(CMO_p rt)
{
  for (size_t i = 0; i < rt->r_obs.size(); ++i) free_read_ob(rt->r_obs[i]);
  for (size_t i = 0; i < rt->w_obs.size(); ++i) free_write_ob(rt->w_obs[i]);
  for (size_t i = 0; i < rt->nobs.size(); ++i) free_nob(rt->nobs[i]);
  delete rt;
  fclose(fp);
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
  ob->shadow_mem = rt->cur_ob;
  rt->cur_ob+=len;
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
  ob->shadow_mem = rt->cur_ob_rw;
  rt->cur_ob_rw+=len;
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
  nob->shadow_mem = rt->cur_nob;
  rt->cur_nob+=len;
  rt->nobs.push_back(nob);
  rt->meta_pos+=1024;
  return nob;
}

void begin_leaky_sec(CMO_p rt) { 
  //TODO size check
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
}
int32_t max_read_ob_shadow_mem_size(CMO_p _rt, ReadObIterator_p ob)
{
  return min(ob->len, ob->len - ob->shadow_mem_pos);
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
}

void end_tx(CMO_p rt)
{
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
  fprintf(fp,"in %s result=%d\n",__func__,ob->rt->g_shadow_mem[cal_ob(ob->shadow_mem+ob->iter_pos)]);
  int32_t data = ob->rt->g_shadow_mem[cal_ob(ob->shadow_mem+ob->iter_pos++)];
  if (ob->iter_pos == ob->shadow_mem_len &&
      ob->shadow_mem_pos + ob->shadow_mem_len < ob->len) {
    end_tx(ob->rt);
    begin_tx(ob->rt);
  }
  return data;
}

void ob_write_next(WriteObIterator_p ob, int32_t data)
{
  fprintf(fp,"in %s data=%d\n",__func__,data);
  ob->rt->g_shadow_mem[cal_ob_rw(ob->shadow_mem+ob->iter_pos++)] = data;
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
  fprintf(fp,"in %s add=%d,result=%d\n",__func__,addr,nob->rt->g_shadow_mem[cal_nob(nob->shadow_mem+addr)]);
  return  nob->rt->g_shadow_mem[cal_nob(nob->shadow_mem+addr)];
}
void nob_write_at(NobArray_p nob, int32_t addr, int32_t data)
{
  fprintf(fp,"in %s add=%d,data=%d\n",__func__,addr,data),
  nob->rt->g_shadow_mem[cal_nob(nob->shadow_mem+addr)] = data;
}

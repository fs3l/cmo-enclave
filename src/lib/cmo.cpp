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

CMO_p init_cmo_runtime() { return new CMO_t; }
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
  ReadObIterator_p ob = new ReadObIterator_t;
  ob->rt = rt;
  ob->data = data;
  ob->len = len;
  ob->shadow_mem = NULL;
  ob->shadow_mem_len = ob->shadow_mem_pos = ob->iter_pos = 0;
  rt->r_obs.push_back(ob);
  return ob;
}

WriteObIterator_p init_write_ob_iterator(CMO_p rt, int32_t *data, int32_t len)
{
  WriteObIterator_p ob = new WriteObIterator_t;
  ob->rt = rt;
  ob->data = data;
  ob->len = len;
  ob->shadow_mem = NULL;
  ob->shadow_mem_len = ob->shadow_mem_pos = ob->iter_pos = 0;
  rt->w_obs.push_back(ob);
  return ob;
}

NobArray_p init_nob_array(CMO_p rt, int32_t *data, int32_t len)
{
  NobArray_p nob = new NobArray_t;
  nob->rt = rt;
  nob->data = data;
  nob->len = len;
  nob->shadow_mem = NULL;
  rt->nobs.push_back(nob);
  return nob;
}

void begin_leaky_sec(CMO_p rt) { begin_tx(rt); }
void end_leaky_sec(CMO_p rt) { end_tx(rt); }
int32_t max_ob_shadow_mem_size(CMO_p _rt, ReadObIterator_p ob)
{
  return min(ob->len, ob->len - ob->shadow_mem_pos);
}
int32_t max_rw_ob_shadow_mem_size(CMO_p _rt, WriteObIterator_p ob)
{
  return min(2, ob->len - ob->shadow_mem_pos);
}

void begin_tx(CMO_p rt)
{
  for (size_t i = 0; i < rt->r_obs.size(); ++i) {
    ReadObIterator_p ob = rt->r_obs[i];
    ob->shadow_mem_len = max_ob_shadow_mem_size(rt, ob);
    ob->shadow_mem = new int32_t[ob->shadow_mem_len];
    ob->iter_pos = 0;
    memcpy(ob->shadow_mem, ob->data + ob->shadow_mem_pos,
           sizeof(int32_t) * ob->shadow_mem_len);
  }

  for (size_t i = 0; i < rt->w_obs.size(); ++i) {
    WriteObIterator_p ob = rt->w_obs[i];
    ob->shadow_mem_len = max_rw_ob_shadow_mem_size(rt, ob);
    ob->shadow_mem = new int32_t[ob->shadow_mem_len];
    ob->iter_pos = 0;
    memcpy(ob->shadow_mem, ob->data + ob->shadow_mem_pos,
           sizeof(int32_t) * ob->shadow_mem_len);
  }

  for (size_t i = 0; i < rt->nobs.size(); ++i) {
    NobArray_p nob = rt->nobs[i];
    nob->shadow_mem = new int32_t[nob->len];
    memcpy(nob->shadow_mem, nob->data, sizeof(int32_t) * nob->len);
  }
}

void end_tx(CMO_p rt)
{
  for (size_t i = 0; i < rt->r_obs.size(); ++i) {
    ReadObIterator_p ob = rt->r_obs[i];
    ob->shadow_mem_pos += ob->iter_pos;
    delete[] ob->shadow_mem;
    ob->shadow_mem = NULL;
    ob->shadow_mem_len = 0;
  }

  for (size_t i = 0; i < rt->w_obs.size(); ++i) {
    WriteObIterator_p ob = rt->w_obs[i];
    memcpy(ob->data + ob->shadow_mem_pos, ob->shadow_mem,
           sizeof(int32_t) * ob->shadow_mem_len);
    ob->shadow_mem_pos += ob->iter_pos;
    delete[] ob->shadow_mem;
    ob->shadow_mem = NULL;
    ob->shadow_mem_len = 0;
  }

  for (size_t i = 0; i < rt->nobs.size(); ++i) {
    NobArray_p nob = rt->nobs[i];
    memcpy(nob->data, nob->shadow_mem, sizeof(int32_t) * nob->len);
    delete[] nob->shadow_mem;
    nob->shadow_mem = NULL;
  }
}

void free_read_ob(ReadObIterator_p ob)
{
  if (ob->shadow_mem != NULL) delete[] ob->shadow_mem;
  delete ob;
}

void free_write_ob(WriteObIterator_p ob)
{
  if (ob->shadow_mem != NULL) delete[] ob->shadow_mem;
  delete ob;
}

void free_nob(NobArray_p nob)
{
  if (nob->shadow_mem != NULL) delete[] nob->shadow_mem;
  delete nob;
}

int32_t ob_read_next(ReadObIterator_p ob)
{
  int32_t data = ob->shadow_mem[ob->iter_pos++];
  if (ob->iter_pos == ob->shadow_mem_len &&
      ob->shadow_mem_pos + ob->shadow_mem_len < ob->len) {
    end_tx(ob->rt);
    begin_tx(ob->rt);
  }
  return data;
}

void ob_write_next(WriteObIterator_p ob, int32_t data)
{
  ob->shadow_mem[ob->iter_pos++] = data;
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
  return nob->shadow_mem[addr];
}
void nob_write_at(NobArray_p nob, int32_t addr, int32_t data)
{
  nob->shadow_mem[addr] = data;
}

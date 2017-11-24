#include "cmo.h"
#include "utils.h"

#include <stdlib.h>
#include <string.h>

// private functions
void begin_tx(CMO_p rt);
void end_tx(CMO_p rt);
void free_read_ob(ReadObIterator_p ob);
void free_write_ob(WriteObIterator_p ob);
void free_nob(NobArray_p nob);
int32_t max_read_ob_buf_size(CMO_p rt, ReadObIterator_p ob);
int32_t max_write_ob_buf_size(CMO_p rt, WriteObIterator_p ob);

CMO_p init_cmo_runtime() { return new CMO_t; }
void free_cmo_runtime(CMO_p rt)
{
  for (size_t i = 0; i < rt->r_obs.size(); ++i) free_read_ob(rt->r_obs[i]);
  for (size_t i = 0; i < rt->w_obs.size(); ++i) free_write_ob(rt->w_obs[i]);
  for (size_t i = 0; i < rt->nobs.size(); ++i) free_nob(rt->nobs[i]);
  delete rt;
}

ReadObIterator_p init_ob_iterator(CMO_p rt, int32_t *data, int32_t len)
{
  ReadObIterator_p ob = new ReadObIterator_t;
  ob->rt = rt;
  ob->data = data;
  ob->len = len;
  ob->buf = NULL;
  ob->buf_len = ob->buf_pos = ob->iter_pos = 0;
  return ob;
}

WriteObIterator_p init_ob_rw_iterator(CMO_p rt, int32_t *data, int32_t len)
{
  WriteObIterator_p ob = new WriteObIterator_t;
  ob->rt = rt;
  ob->data = data;
  ob->len = len;
  ob->buf = NULL;
  ob->buf_len = ob->buf_pos = ob->iter_pos = 0;
  return ob;
}

NobArray_p init_nob_array(CMO_p rt, int32_t *data, int32_t len)
{
  NobArray_p nob = new NobArray_t;
  nob->rt = rt;
  nob->data = data;
  nob->len = len;
  nob->buf = NULL;
  return nob;
}

void begin_leaky_sec(CMO_p rt) { begin_tx(rt); }
void end_leaky_sec(CMO_p rt) { end_tx(rt); }
int32_t max_ob_buf_size(CMO_p _rt, ReadObIterator_p ob)
{
  return min(ob->len, ob->len - ob->buf_pos);
}
int32_t max_rw_ob_buf_size(CMO_p _rt, WriteObIterator_p ob)
{
  return min(2, ob->len - ob->buf_pos);
}

void begin_tx(CMO_p rt)
{
  for (size_t i = 0; i < rt->r_obs.size(); ++i) {
    ReadObIterator_p ob = rt->r_obs[i];
    ob->buf_len = max_ob_buf_size(rt, ob);
    ob->buf = new int32_t[ob->buf_len];
    ob->iter_pos = 0;
    memcpy(ob->buf, ob->data + ob->buf_pos, sizeof(int32_t) * ob->buf_len);
  }

  for (size_t i = 0; i < rt->w_obs.size(); ++i) {
    WriteObIterator_p ob = rt->w_obs[i];
    ob->buf_len = max_rw_ob_buf_size(rt, ob);
    ob->buf = new int32_t[ob->buf_len];
    ob->iter_pos = 0;
    memcpy(ob->buf, ob->data + ob->buf_pos, sizeof(int32_t) * ob->buf_len);
  }

  for (size_t i = 0; i < rt->nobs.size(); ++i) {
    NobArray_p nob = rt->nobs[i];
    nob->buf = new int32_t[nob->len];
    memcpy(nob->buf, nob->data, sizeof(int32_t) * nob->len);
  }
}

void end_tx(CMO_p rt)
{
  for (size_t i = 0; i < rt->r_obs.size(); ++i) {
    ReadObIterator_p ob = rt->r_obs[i];
    memcpy(ob->data + ob->buf_pos, ob->buf, sizeof(int32_t) * ob->buf_len);
    ob->buf_pos += ob->iter_pos;
    delete[] ob->buf;
    ob->buf = NULL;
    ob->buf_len = 0;
  }

  for (size_t i = 0; i < rt->w_obs.size(); ++i) {
    WriteObIterator_p ob = rt->w_obs[i];
    memcpy(ob->data + ob->buf_pos, ob->buf, sizeof(int32_t) * ob->buf_len);
    ob->buf_pos += ob->iter_pos;
    delete[] ob->buf;
    ob->buf = NULL;
    ob->buf_len = 0;
  }

  for (size_t i = 0; i < rt->nobs.size(); ++i) {
    NobArray_p nob = rt->nobs[i];
    memcpy(nob->data, nob->buf, sizeof(int32_t) * nob->len);
    delete[] nob->buf;
    nob->buf = NULL;
  }
}

void free_read_ob(ReadObIterator_p ob)
{
  if (ob->buf != NULL) delete ob->buf;
  delete ob;
}

void free_write_ob(WriteObIterator_p ob)
{
  if (ob->buf != NULL) delete ob->buf;
  delete ob;
}

void free_nob(NobArray_p nob)
{
  if (nob->buf != NULL) delete nob->buf;
  delete nob;
}

int32_t ob_read_next(ReadObIterator_p ob)
{
  int32_t data = ob->buf[ob->iter_pos++];
  if (ob->iter_pos == ob->buf_len && ob->buf_pos + ob->buf_len < ob->len) {
    end_tx(ob->rt);
    begin_tx(ob->rt);
  }
  return data;
}

int32_t ob_write_next(WriteObIterator_p ob)
{
  int32_t data = ob->buf[ob->iter_pos++];
  if (ob->iter_pos == ob->buf_len && ob->buf_pos + ob->buf_len < ob->len) {
    end_tx(ob->rt);
    begin_tx(ob->rt);
  }
  return data;
}

int32_t nob_read_at(NobArray_p nob, int32_t addr) { return nob->buf[addr]; }
void nob_write_at(NobArray_p nob, int32_t addr, int32_t data)
{
  nob->buf[addr] = data;
}

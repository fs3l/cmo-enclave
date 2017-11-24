#ifndef CMO_H
#define CMO_H

#include <stdint.h>
#include <vector>

struct ReadObIterator;
struct WriteObIterator;
struct NobArray;

struct CMO {
  std::vector<struct ReadObIterator *> r_obs;
  std::vector<struct WriteObIterator *> w_obs;
  std::vector<struct NobArray *> nobs;
};
typedef struct CMO CMO_t;
typedef struct CMO *CMO_p;

CMO_p init_cmo_runtime();
void free_cmo_runtime(CMO_p rt);

struct ReadObIterator* init_read_ob_iterator(CMO_p rt, int32_t *data, int32_t len);
struct WriteObIterator* init_write_ob_iterator(CMO_p rt, int32_t *data, int32_t len);
struct NobArray* init_nob_array(CMO_p rt, int32_t *data, int32_t len);
void begin_leaky_sec(CMO_p rt);
void end_leaky_sec(CMO_p rt);

struct ReadObIterator {
  CMO_p rt;
  int32_t* data;
  int32_t len;
  int32_t* shadow_mem;
  int32_t shadow_mem_len, shadow_mem_pos, iter_pos;
};
typedef struct ReadObIterator ReadObIterator_t;
typedef struct ReadObIterator* ReadObIterator_p;

int32_t ob_read_next(ReadObIterator_p ob);

struct WriteObIterator {
  CMO_p rt;
  int32_t* data;
  int32_t len;
  int32_t* shadow_mem;
  int32_t shadow_mem_len, shadow_mem_pos, iter_pos;
};
typedef struct WriteObIterator WriteObIterator_t;
typedef struct WriteObIterator* WriteObIterator_p;

void ob_write_next(WriteObIterator_p ob, int32_t data);

struct NobArray {
  CMO_p rt;
  int32_t* data;
  int32_t len;
  int32_t* shadow_mem;
};
typedef struct NobArray NobArray_t;
typedef struct NobArray* NobArray_p;

int32_t nob_read_at(NobArray_p nob, int32_t addr);
void nob_write_at(NobArray_p nob, int32_t addr, int32_t data);

#endif

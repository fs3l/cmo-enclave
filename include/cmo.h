#ifndef CMO_H
#define CMO_H

#include <stdint.h>
#include <vector>

#define PFO 1
#define L1_WAYS 8
#define L1_SETS 64

struct ReadObIterator;
struct WriteObIterator;
struct NobArray;
struct ReadNobArray;
#if PFO
struct L1 {
   char counts[L1_SETS];
};
typedef struct L1* L1_p;
#endif

extern uint32_t shadow_mem[2 * 1024 * 1024] __attribute__((aligned(4096)));
struct CMO {
  std::vector<struct ReadObIterator*> r_obs;
  std::vector<struct WriteObIterator*> w_obs;
  std::vector<struct NobArray*> nobs;
  std::vector<struct ReadNobArray*> r_nobs;
  uint32_t* g_shadow_mem;
  int32_t meta_pos;
  int32_t cur_ob;
  int32_t cur_ob_rw;
  int32_t cur_nob;
#if PFO
  L1_p l1counts;
#endif
};

typedef struct CMO CMO_t;
typedef struct CMO* CMO_p;

struct ALLOC {
  int32_t meta;
  int32_t ob_r;
  int32_t ob_w;
  int32_t nob_r;
  int32_t nob_w;
};

typedef struct ALLOC ALLOC_t;
typedef struct ALLOC* ALLOC_p;

CMO_p init_cmo_runtime();
void free_cmo_runtime(CMO_p rt);
void cmo_abort(CMO_p rt, const char* abort_msg);

struct ReadObIterator* init_read_ob_iterator(CMO_p rt, const int32_t* data,
                                             int32_t len);
struct WriteObIterator* init_write_ob_iterator(CMO_p rt, int32_t* data,
                                               int32_t len);
struct NobArray* init_nob_array(CMO_p rt, int32_t* data, int32_t len);
struct ReadNobArray* init_read_nob_array(CMO_p rt, int32_t* data, int32_t len);
void begin_leaky_sec(CMO_p rt);
void end_leaky_sec(CMO_p rt);

struct ReadObIterator {
  CMO_p rt;
  ALLOC_p alloc;
  const int32_t* data;
  int32_t len;
  int32_t shadow_mem;
  uint32_t* g_shadow_mem;
  int32_t shadow_mem_len, shadow_mem_pos, iter_pos;
};
typedef struct ReadObIterator ReadObIterator_t;
typedef struct ReadObIterator* ReadObIterator_p;

int32_t ob_read_next(ReadObIterator_p ob);
void reset_read_ob(ReadObIterator_p ob);

struct WriteObIterator {
  CMO_p rt;
  ALLOC_p alloc;
  int32_t* data;
  int32_t len;
  int32_t shadow_mem;
  uint32_t* g_shadow_mem;
  int32_t shadow_mem_len, shadow_mem_pos, iter_pos;
};
typedef struct WriteObIterator WriteObIterator_t;
typedef struct WriteObIterator* WriteObIterator_p;

void ob_write_next(WriteObIterator_p ob, int32_t data);
void reset_write_ob(WriteObIterator_p ob);

struct NobArray {
  CMO_p rt;
  ALLOC_p alloc;
  int32_t* data;
  int32_t len;
  int32_t shadow_mem;
  uint32_t* g_shadow_mem;
};
typedef struct NobArray NobArray_t;
typedef struct NobArray* NobArray_p;

int32_t nob_read_at(const NobArray_p nob, int32_t addr);
void nob_write_at(NobArray_p nob, int32_t addr, int32_t data);

struct ReadNobArray {
  CMO_p rt;
  ALLOC_p alloc;
  int32_t* data;
  int32_t len;
  int32_t shadow_mem;
  uint32_t* g_shadow_mem;
};

typedef struct ReadNobArray ReadNobArray_t;
typedef struct ReadNobArray* ReadNobArray_p;

int32_t nob_read_at(const ReadNobArray_p nob, int32_t addr);

#endif

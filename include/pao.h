#ifndef PAO_H
#define PAO_H

#include <vector>
#include <stdint.h>

struct ArrayRO;
struct ArrayRW;
struct NobArray;
struct ReadNobArray;

struct PAO {
  std::vector<struct ArrayRO*> arrays_ro;
  std::vector<struct ArrayRW*> arrays_rw;
  // the global shadow memory
  // 0-15 -  meta data
  // 16-63 - ob data r
  // 64 - 111 - ob data rw
  // 112 - 639 - nob data
  // 1024*1023 + 512 - 1024*1024-1 stack frame
  uint32_t g_shadow_mem[2 * 1024 * 1024] __attribute__((aligned(4096)));
  // the free slot for meta, currently, we allocated one cache line for one
  // ob/nob object
  int32_t meta_pos;
  int32_t cur_array_ro;
  int32_t cur_array_rw;
};
typedef struct PAO PAO_t;
typedef struct PAO* PAO_p;

PAO_p init_pao_runtime();
void free_pao_runtime(PAO_p rt);
void pao_abort(PAO_p rt, const char* abort_msg);

struct ArrayRO* init_array_ro(PAO_p rt, const int32_t* data,
                                             int32_t len);
struct ArrayRW* init_array_rw(PAO_p rt, int32_t* data,
                                               int32_t len);
void begin_leaky_sec(PAO_p rt);
void end_leaky_sec(PAO_p rt);

struct ArrayRO {
  PAO_p rt;
  const int32_t* data;
  int32_t len;
  int32_t shadow_mem;
  uint32_t* g_shadow_mem;
  int32_t shadow_mem_len, shadow_mem_pos, iter_pos;
};
typedef struct ArrayRO ArrayRO_t;
typedef struct ArrayRO* ArrayRO_p;

int32_t read_at(ArrayRO_p ob, int32_t idx);

struct ArrayRW {
  PAO_p rt;
  int32_t* data;
  int32_t len;
  int32_t shadow_mem;
  uint32_t* g_shadow_mem;
  int32_t shadow_mem_len, shadow_mem_pos, iter_pos;
};
typedef struct ArrayRW ArrayRW_t;
typedef struct ArrayRW* ArrayRW_p;

void write_at(ArrayRW_p ob, int32_t idx, int32_t data);
int32_t read_at(ArrayRW_p ob, int32_t idx);
#endif

#ifndef CMO_MEMPOOL_H
#define CMO_MEMPOOL_H

#include "cmo.h"
#include "cmo_mem.h"

/**
 * MemoryPool consists of two linked list, i.e. free block list and used block
 * list
 *
 * data layout:
 *   meta data:
 *     data array length       [16 bits]
 *     max capacity            [16 bits]
 *     (if begin/end address ==0, it means empty list)
 *     free block:
 *       begin address         [16 bits]
 *       end address           [16 bits]
 *     used block:
 *       begin address         [16 bits]
 *       end address           [16 bits]
 *   block data:
 *     ...
 *     block meta data: (address == 0 means NULL, i.e. first or last element)
 *       address to prev block [16 bits]
 *       address to next block [16 bits]
 *     payload                 [sizeof(element) aligned with 32bits]
 *     ...
 */

template <class T>
class MemeryPool
{
  const int32_t FREE_BLOCK_META_ADDR = 1;
  const int32_t USED_BLOCK_META_ADDR = 2;

public:
  MemeryPool(CMO_p rt, int32_t capacity)
  {
    if (capacity > UINT16_MAX) cmo_abort(rt, "fail to initialize MemeryPool");
    const int32_t element_block_size = calc_element_block_size<T>();
    int32_t data_len = 3;                             // meta data
    data_len += capacity * (1 + element_block_size);  // block data

    if (data_len > UINT16_MAX) cmo_abort(rt, "fail to initialize MemeryPool");
    data = new int32_t[data_len];

    // set meta data
    data[0] = (data_len << 16) | capacity;
    data[1] = (3 << 16) | (data_len - 1 - element_block_size);
    data[2] = 0;
    // set block data;
    for (int32_t i = 0, block_addr = 3; i < capacity; ++i) {
      int32_t prev_addr = block_addr - (1 + element_block_size);
      int32_t next_addr = block_addr + (1 + element_block_size);
      if (i == 0)
        prev_addr = 0;
      else if (i == capacity - 1)
        next_addr = 0;
      data[block_addr] = (prev_addr << 16) | next_addr;
      block_addr = next_addr;
    }
    nob = init_nob_array(rt, data, data_len);
  }
  ~MemeryPool() { delete[] data; }
  void reset_nob(CMO_p rt) { nob = init_nob_array(rt, data, data[0] >> 16); }
  // return true if the pool is empty
  bool empty() const { return get_meta_begin_addr(USED_BLOCK_META_ADDR) == 0; }
  // return true if the pool is full
  bool full() const { return get_meta_begin_addr(FREE_BLOCK_META_ADDR) == 0; }
  // allocate a block and return its address
  int32_t alloc_block()
  {
    int32_t addr = get_meta_begin_addr(FREE_BLOCK_META_ADDR);
    int32_t next_addr = get_block_next_addr(addr);
    if (next_addr == 0) {  // no more free block
      set_meta_addr(FREE_BLOCK_META_ADDR, 0, 0);
    } else {
      set_block_prev_addr(next_addr, 0);
      set_meta_begin_addr(FREE_BLOCK_META_ADDR, next_addr);
    }
    int32_t used_addr = get_meta_end_addr(USED_BLOCK_META_ADDR);
    if (used_addr == 0) {  // this is the first allocated block
      set_block_addr(addr, 0, 0);
      set_meta_addr(USED_BLOCK_META_ADDR, addr, addr);
    } else {
      set_block_next_addr(used_addr, addr);
      set_block_addr(addr, used_addr, 0);
      set_meta_end_addr(USED_BLOCK_META_ADDR, addr);
    }
    return addr;
  }
  // free a block
  void free_block(int32_t addr)
  {
    int32_t prev_addr = get_block_prev_addr(addr);
    int32_t next_addr = get_block_next_addr(addr);
    if (prev_addr == 0 &&
        next_addr == 0) {  // this is the only block in the used list
      set_block_addr(USED_BLOCK_META_ADDR, 0, 0);
    } else {
      if (prev_addr == 0) {  // this is the first block in the used list
        set_meta_begin_addr(USED_BLOCK_META_ADDR, next_addr);
        set_block_prev_addr(next_addr, 0);
      } else {
        set_block_prev_addr(next_addr, prev_addr);
      }
      if (next_addr == 0) {  // this is the last block in the used list
        set_meta_end_addr(USED_BLOCK_META_ADDR, prev_addr);
        set_block_next_addr(prev_addr, 0);
      } else {
        set_block_next_addr(prev_addr, next_addr);
      }
    }
    int32_t begin_addr = get_meta_begin_addr(FREE_BLOCK_META_ADDR);
    if (begin_addr == 0) {  // was full
      set_block_addr(addr, 0, 0);
      set_meta_addr(FREE_BLOCK_META_ADDR, addr, addr);
    } else {
      set_block_prev_addr(begin_addr, addr);
      set_block_addr(addr, 0, begin_addr);
      set_meta_begin_addr(FREE_BLOCK_META_ADDR, addr);
    }
  }
  // read block
  template <class E>
  void read_block(int32_t addr, E* element) const
  {
    read_element<E>(nob, addr + 1, element);
  }
  // write block
  template <class E>
  void write_block(int32_t addr, const E* element)
  {
    write_element<E>(nob, addr + 1, element);
  }

private:
  int32_t get_meta_begin_addr(int32_t meta_addr) const
  {
    return nob_read_at(nob, meta_addr) >> 16;
  }
  int32_t get_meta_end_addr(int32_t meta_addr) const
  {
    return nob_read_at(nob, meta_addr) & 0xffff;
  }
  void set_meta_addr(int32_t meta_addr, int32_t begin_addr, int32_t end_addr)
  {
    nob_write_at(nob, meta_addr, (begin_addr << 16) | end_addr);
  }
  void set_meta_begin_addr(int32_t meta_addr, int32_t begin_addr)
  {
    set_meta_addr(meta_addr, begin_addr, get_meta_end_addr(meta_addr));
  }
  void set_meta_end_addr(int32_t meta_addr, int32_t end_addr)
  {
    set_meta_addr(meta_addr, get_meta_begin_addr(meta_addr), end_addr);
  }
  int32_t get_block_prev_addr(int32_t block_addr) const
  {
    return nob_read_at(nob, block_addr) >> 16;
  }
  int32_t get_block_next_addr(int32_t block_addr) const
  {
    return nob_read_at(nob, block_addr) & 0xffff;
  }
  void set_block_addr(int32_t block_addr, int32_t prev_addr, int32_t next_addr)
  {
    nob_write_at(nob, block_addr, (prev_addr << 16) | next_addr);
  }
  void set_block_prev_addr(int32_t block_addr, int32_t prev_addr)
  {
    set_block_addr(block_addr, prev_addr, get_block_next_addr(block_addr));
  }
  void set_block_next_addr(int32_t block_addr, int32_t next_addr)
  {
    set_block_addr(block_addr, get_block_prev_addr(block_addr), next_addr);
  }

  int32_t* data;
  NobArray_p nob;
};

#endif

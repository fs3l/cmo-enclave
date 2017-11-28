#ifndef CMO_QUEUE_H
#define CMO_QUEUE_H

#include "cmo.h"

// compute the number of int32_t blocks required to store an element
template <class T>
int32_t calc_element_block_size()
{
  const int32_t element_bytes = sizeof(T);
  const int32_t element_block_bytes =
      (element_bytes % sizeof(int32_t) == 0)
          ? element_bytes
          : element_bytes - element_bytes % sizeof(int32_t) + sizeof(int32_t);
  return element_block_bytes / sizeof(int32_t);
}

template <class T>
void read_element(const NobArray_p nob, int32_t addr, T* element)
{
  int32_t i, bytes;
  unsigned char* dest = (unsigned char*)(element);
  for (i = 0; i < sizeof(T) / sizeof(int32_t); ++i) {
    *(int32_t*)dest = nob_read_at(nob, addr + i);
    dest += sizeof(int32_t);
    bytes += sizeof(int32_t);
  }
  if (bytes < sizeof(T)) {
    int32_t data = nob_read_at(nob, addr + i);
    while (bytes < sizeof(T)) {
      *dest = (unsigned char)(data & 0xff);
      data >>= 8;
      dest += sizeof(unsigned char);
      bytes += sizeof(unsigned char);
    }
  }
}

template <class T>
void write_element(NobArray_p nob, int32_t addr, T* element)
{
  int32_t i, bytes;
  unsigned char* src = (unsigned char*)(element);
  for (i = 0; i < sizeof(T) / sizeof(int32_t); ++i) {
    nob_write_at(nob, addr + i, *(int32_t*)src);
    src += sizeof(int32_t);
    bytes += sizeof(int32_t);
  }
  if (bytes < sizeof(T)) {
    int32_t data = 0;
    src = (unsigned char*)(element) + sizeof(T) - sizeof(unsigned char);
    while (bytes < sizeof(T)) {
      data = (data << 8) | (*src & 0xff);
      src -= sizeof(unsigned char);
      bytes += sizeof(unsigned char);
    }
    nob_write_at(nob, addr + i, data);
  }
}

/**
 * data layout:
 *   max capacity of the queue        [16bit]
 *   element block size               [16bit]
 *   current length                   [16bit]
 *   the index to the first element   [16bit]
 *   data...                          [sizeof(element) aligned with 32bits] *
 * capacity
 */

template <class T>
class SimpleQueue
{
public:
  SimpleQueue(CMO_p rt, int32_t capacity)
  {
    if (capacity > UINT16_MAX) cmo_abort(rt, "fail to initialize SimpleQueue");

    const int32_t element_block_size = calc_element_block_size<T>();
    int32_t data_len = 2;  // global meta data
    data_len += capacity * element_block_size;

    data = new int32_t[data_len];
    data[0] = capacity << 16 | element_block_size;
    data[1] = 0;
    nob = init_nob_array(rt, data, data_len);
  }
  ~SimpleQueue() { delete[] data; }
  int32_t capacity() const { return nob_read_at(nob, 0) >> 16; }
  int32_t element_block_size() const { return nob_read_at(nob, 0) & 0xffff; }
  int32_t size() const { return nob_read_at(nob, 1) >> 16; }
  int32_t front_idx() const { return nob_read_at(nob, 1) & 0xffff; }
  bool empty() const { return size() == 0; }
  bool full() const { return size() == capacity(); }
  void enqueue(T element)
  {
    int32_t idx = front_idx();
    int32_t len = size();
    int32_t addr = ((idx + len) % capacity()) * element_block_size() + 2;
    write_element<T>(nob, addr, &element);
    nob_write_at(nob, 1, (len + 1) << 16 | idx);
  }
  void front(T* element) const
  {
    int32_t addr = front_idx() * element_block_size() + 2;
    read_element<T>(nob, addr, element);
  }
  void dequeue()
  {
    int32_t idx = front_idx();
    int32_t len = size();
    idx = (idx + 1) % capacity();
    nob_write_at(nob, 1, (len - 1) << 16 | idx);
  }

private:
  int32_t* data;
  NobArray_p nob;
};

/**
 * MultiQueue stores each queue (including a free block queue) as a double
 * linked list.
 *
 * data layout:
 *   global meta data:
 *     data array length       [32 bits]
 *     number of queues        [16 bits]
 *     max capacity            [16 bits]
 *   queue meta data: (if begin/end address == 0, it means empty list)
 *     free block: (queue_id = -1)
 *       begin address         [16 bits]
 *       end address           [16 bits]
 *     queue 1:    (queue_id = 0)
 *       begin address         [16 bits]
 *       end address           [16 bits]
 *     ...
 *     queue n:    (queue_id = n - 1)
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
class MultiQueue
{
public:
  MultiQueue(CMO_p rt, int32_t capacity, int32_t num_of_queue)
  {
    if (capacity > UINT16_MAX || num_of_queue > UINT16_MAX)
      cmo_abort(rt, "fail to initialize MultiQueue");

    const int32_t element_block_size = calc_element_block_size<T>();
    int32_t data_len = 2;                             // global meta data
    data_len += 1 + num_of_queue;                     // queue meta data;
    data_len += capacity * (1 + element_block_size);  // block data;

    if (data_len > UINT16_MAX) cmo_abort(rt, "fail to initialize MultiQueue");
    data = new int32_t[data_len];

    // set global meta data
    data[0] = data_len;
    data[1] = (num_of_queue << 16) | capacity;
    // set queue meta data
    data[2] = ((3 + num_of_queue) << 16) | (data_len - 1 - element_block_size);
    for (int32_t i = 0; i < num_of_queue; ++i) data[3 + i] = 0;
    // set block data
    for (int32_t i = 0, block_addr = 3 + num_of_queue; i < capacity; ++i) {
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
  ~MultiQueue() { delete[] data; }
  void reset_nob(CMO_p rt) { nob = init_nob_array(rt, data, data[0]); }
  // return the size of the queue queue_id
  int32_t size(int32_t queue_id) const
  {
    int32_t addr = get_queue_begin_addr(queue_id);
    int32_t size = 0;
    while (addr != 0) {
      ++size;
      addr = get_block_next_addr(addr);
    }
    return size;
  }

  // return true if the queue queue_id is empty
  bool empty(int32_t queue_id) const
  {
    return get_queue_begin_addr(queue_id) == 0;
  }

  // return true if the entire MultiQueue is full, i.e., no more free block
  bool full() const { return empty(-1); }
  // return the first element in the queue queue_id
  void front(int32_t queue_id, T* element) const
  {
    int32_t addr = get_queue_begin_addr(queue_id);
    read_element<T>(nob, addr + 1, element);
  }

  // return the last element in the queue queue_id
  void back(int32_t queue_id, T* element) const
  {
    int32_t addr = get_queue_end_addr(queue_id);
    read_element<T>(nob, addr + 1, element);
  }

  // push an element in the front of the queue queue_id
  void push_front(int32_t queue_id, T element)
  {
    int32_t addr = alloc_block();
    write_element<T>(nob, addr + 1, &element);
    int32_t begin_addr = get_queue_begin_addr(queue_id);
    if (begin_addr == 0) {  // was empty queue
      set_block_addr(addr, 0, 0);
      set_queue_addr(queue_id, addr, addr);
    } else {
      set_block_prev_addr(begin_addr, addr);
      set_block_addr(addr, 0, begin_addr);
      set_queue_begin_addr(queue_id, addr);
    }
  }

  // push an element in the back of the queue queue_id
  void push_back(int32_t queue_id, T element)
  {
    int32_t addr = alloc_block();
    write_element<T>(nob, addr + 1, &element);
    int32_t end_addr = get_queue_end_addr(queue_id);
    if (end_addr == 0) {  // was empty queue
      set_block_addr(addr, 0, 0);
      set_queue_addr(queue_id, addr, addr);
    } else {
      set_block_next_addr(end_addr, addr);
      set_block_addr(addr, end_addr, 0);
      set_queue_end_addr(queue_id, addr);
    }
  }

  // pop the element in the front of the queue queue_id
  void pop_front(int32_t queue_id)
  {
    int32_t addr = get_queue_begin_addr(queue_id);
    int32_t next_addr = get_block_next_addr(addr);
    if (next_addr == 0) {  // now empty queue
      set_queue_addr(queue_id, 0, 0);
    } else {
      set_block_prev_addr(next_addr, 0);
      set_queue_begin_addr(queue_id, next_addr);
    }
    free_block(addr);
  }

  // pop the element in the back of the queue queue_id
  void pop_back(int32_t queue_id)
  {
    int32_t addr = get_queue_end_addr(queue_id);
    int32_t prev_addr = get_block_prev_addr(addr);
    if (prev_addr == 0) {  // now empty queue
      set_queue_addr(queue_id, 0, 0);
    } else {
      set_block_next_addr(prev_addr, 0);
      set_queue_end_addr(queue_id, prev_addr);
    }
    free_block(addr);
  }

private:
  int32_t get_queue_begin_addr(int32_t queue_id) const
  {
    return nob_read_at(nob, 3 + queue_id) >> 16;
  }
  int32_t get_queue_end_addr(int32_t queue_id) const
  {
    return nob_read_at(nob, 3 + queue_id) & 0xffff;
  }
  void set_queue_addr(int32_t queue_id, int32_t begin_addr, int32_t end_addr)
  {
    nob_write_at(nob, 3 + queue_id, (begin_addr << 16) | end_addr);
  }
  void set_queue_begin_addr(int32_t queue_id, int32_t begin_addr)
  {
    set_queue_addr(queue_id, begin_addr, get_queue_end_addr(queue_id));
  }
  void set_queue_end_addr(int32_t queue_id, int32_t end_addr)
  {
    set_queue_addr(queue_id, get_queue_begin_addr(queue_id), end_addr);
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
  int32_t alloc_block()
  {
    int32_t free_addr = get_queue_begin_addr(-1);
    int32_t next_addr = get_block_next_addr(free_addr);
    if (next_addr == 0) {  // no more free block
      set_queue_addr(-1, 0, 0);
    } else {
      set_block_prev_addr(next_addr, 0);
      set_queue_begin_addr(-1, next_addr);
    }
    return free_addr;
  }
  void free_block(int32_t addr)
  {
    int32_t begin_addr = get_queue_begin_addr(-1);
    if (begin_addr == 0) {  // was full
      set_block_addr(addr, 0, 0);
      set_queue_addr(-1, addr, addr);
    } else {
      set_block_prev_addr(begin_addr, addr);
      set_block_addr(addr, 0, begin_addr);
      set_queue_begin_addr(-1, addr);
    }
  }

  int32_t* data;
  NobArray_p nob;
};

#endif

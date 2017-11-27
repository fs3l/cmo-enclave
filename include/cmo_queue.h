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

#endif

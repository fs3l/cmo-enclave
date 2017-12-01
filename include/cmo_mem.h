#ifndef CMO_MEM_H
#define CMO_MEM_H

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

// read an element from nob at address addr.
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

// write an element to nob at address addr.
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

#endif

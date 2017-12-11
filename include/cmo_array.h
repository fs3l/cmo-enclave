#ifndef CMO_ARRAY_H
#define CMO_ARRAY_H

#include "cmo.h"
#include "cmo_mem.h"

/**
 * Array data layout:
 *  data array length [32 bits]
 *  array size        [32 bits]
 *  element           [sizeof(element) aligned with 32bits]
 *  ...
 */

template <class T>
class Array
{
public:
  Array(CMO_p rt, int32_t size)
  {
    const int32_t element_block_size = calc_element_block_size<T>();
    int32_t data_len = 2;  // meta data
    data_len += size * element_block_size;

    data = new int32_t[data_len];
    data[0] = data_len;
    data[1] = size;
    nob = init_nob_array(rt, data, data_len);
  }
  ~Array() { delete[] data; }
  void reset_nob(CMO_p rt) { nob = init_nob_array(rt, data, data[0]); }
  // return the size of the array
  int32_t size() const { return nob_read_at(nob, 1); }
  // read element at index idx
  void read(int32_t idx, T* element) const
  {
    const int32_t element_block_size = calc_element_block_size<T>();
    int32_t addr = idx * element_block_size + 2;
    read_element<T>(nob, addr, element);
  }
  // write element at index idx
  void write(int32_t idx, const T* element)
  {
    const int32_t element_block_size = calc_element_block_size<T>();
    int32_t addr = idx * element_block_size + 2;
    write_element<T>(nob, addr, element);
  }
  // read element at index idx (leaky)
  void read_leaky(int32_t idx, T* element) const
  {
    const int32_t element_block_size = calc_element_block_size<T>();
    int32_t addr = idx * element_block_size + 2;
    read_element<T>(data + addr, element);
  }
  // write element at index idx (leaky)
  void write_leaky(int32_t idx, const T* element)
  {
    const int32_t element_block_size = calc_element_block_size<T>();
    int32_t addr = idx * element_block_size + 2;
    write_element<T>(data + addr, element);
  }

  int32_t* data;
  NobArray_p nob;
};

template <class T>
class ROArray
{
public:
  ROArray(CMO_p rt, int32_t size)
  {
    const int32_t element_block_size = calc_element_block_size<T>();
    int32_t data_len = 2;  // meta data
    data_len += size * element_block_size;

    data = new int32_t[data_len];
    data[0] = data_len;
    data[1] = size;
    nob = init_read_nob_array(rt, data, data_len);
  }
  ~ROArray() { delete[] data; }
  void reset_nob(CMO_p rt) { nob = init_read_nob_array(rt, data, data[0]); }
  // return the size of the array
  int32_t size() const { return nob_read_at(nob, 1); }
  // read element at index idx
  void read(int32_t idx, T* element) const
  {
    const int32_t element_block_size = calc_element_block_size<T>();
    int32_t addr = idx * element_block_size + 2;
    read_element<T>(nob, addr, element);
  }
  // read element at index idx (leaky)
  void read_leaky(int32_t idx, T* element) const
  {
    const int32_t element_block_size = calc_element_block_size<T>();
    int32_t addr = idx * element_block_size + 2;
    read_element<T>(data + addr, element);
  }
  // write element at index idx (leaky)
  void write_leaky(int32_t idx, const T* element)
  {
    const int32_t element_block_size = calc_element_block_size<T>();
    int32_t addr = idx * element_block_size + 2;
    write_element<T>(data + addr, element);
  }

  int32_t* data;
  ReadNobArray_p nob;
};

#endif

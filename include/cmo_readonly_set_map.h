#ifndef CMO_READONLY_SET_MAP_H
#define CMO_READONLY_SET_MAP_H

#include "cmo.h"
#include "cmo_array.h"

#include <stdio.h>
#include <stdlib.h>

template <class T>
class ROSet
{
public:
  ROSet(CMO_p rt, int32_t size, const T* _data) : data(rt, size)
  {
    for (int32_t i = 0; i < size; ++i) {
      data.write(i, &_data[i]);
      if (i > 0 && _data[i] < _data[i - 1]) {
        printf("ROSet: invalid data");
        abort();
      }
    }
  }
  ~ROSet() {}
  void reset_ob(CMO_p rt) { data.reset_nob(rt); }
  int32_t size() const { return data.size(); }
  bool find(const T& element) const
  {
    return find_at(element, 0, size() - 1) != -1;
  }

private:
  int32_t find_at(const T& element, int32_t left, int32_t right) const
  {
    if (right >= left) {
      int32_t mid = (left + right) / 2;
      T v;
      data.read(mid, &v);
      if (v == element)
        return mid;
      else if (v > element)
        return find_at(element, left, mid - 1);
      else
        return find_at(element, mid + 1, right);
    } else {
      return -1;
    }
  }

  ROArray<T> data;
};

template <class K, class V>
struct readonly_map_element {
  K key;
  V value;
};

template <class K, class V>
class ROMap
{
public:
  ROMap(CMO_p rt, int32_t size, const K* keys, const V* values) : data(rt, size)
  {
    for (int32_t i = 0; i < size; ++i) {
      struct readonly_map_element<K, V> e;
      e.key = keys[i];
      e.value = values[i];
      data.write(i, &e);
      if (i > 0 && keys[i] < keys[i - 1]) {
        printf("ROMap: invalid data");
        abort();
      }
    }
  }
  ~ROMap() {}
  void reset_ob(CMO_p rt) { data.reset_nob(rt); }
  int32_t size() const { return data.size(); }
  // return -1 if not found.
  int32_t find(const K& key) const { return find_at(key, 0, size() - 1); }
  void get_element(int32_t idx, K* key, V* value) const
  {
    struct readonly_map_element<K, V> e;
    data.read(idx, &e);
    *key = e.key;
    *value = e.value;
  }
  void get_key(int32_t idx, K* key) const
  {
    struct readonly_map_element<K, V> e;
    data.read(idx, &e);
    *key = e.key;
  }
  void get_value(int32_t idx, V* value) const
  {
    struct readonly_map_element<K, V> e;
    data.read(idx, &e);
    *value = e.value;
  }

private:
  int32_t find_at(const K& key, int32_t left, int32_t right) const
  {
    if (right >= left) {
      int32_t mid = (left + right) / 2;
      struct readonly_map_element<K, V> e;
      data.read(mid, &e);
      if (e.key == key)
        return mid;
      else if (e.key > key)
        return find_at(key, left, mid - 1);
      else
        return find_at(key, mid + 1, right);
    } else {
      return -1;
    }
  }

  ROArray<struct readonly_map_element<K, V>> data;
};
#endif
